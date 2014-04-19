/**
  Copyright 2014 Masakazu Kitajo

  Licensed to the Apache Software Foundation (ASF) under one
  or more contributor license agreements.  See the NOTICE file
  distributed with this work for additional information
  regarding copyright ownership.  The ASF licenses this file
  to you under the Apache License, Version 2.0 (the
  "License"); you may not use this file except in compliance
  with the License.  You may obtain a copy of the License at

      http://www.apache.org/licenses/LICENSE-2.0

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.
 */

#include <iostream>
#include <atscppapi/GlobalPlugin.h>
#include <atscppapi/TransactionPlugin.h>
#include <atscppapi/TransformationPlugin.h>
#include <atscppapi/PluginInit.h>
#include <atscppapi/Logger.h>
#include <magic.h>

using namespace atscppapi;
using std::string;

class PeekContext : public Transaction::ContextValue {
public:
    string body_description = "Unknown";
};

class BodyRecognitionPlugin : public TransformationPlugin {
public:
    BodyRecognitionPlugin(Transaction &transaction, Type type)
        : TransformationPlugin(transaction, type), transaction_(transaction) {
    }

    void consume(const string &data) {
        if (0 == recognized_) {
            magic_t magic_ = magic_open(MAGIC_NONE);
            magic_load(magic_, NULL);
            const char *description = magic_buffer(magic_, data.c_str(), data.length());
            if (description == NULL) {
                recognized_ = -1;
            } else {
                shared_ptr<PeekContext> ctx =
                    std::dynamic_pointer_cast<PeekContext>(
                        transaction_.getContextValue("PeekContext"));
                ctx->body_description = description;
                recognized_ = 1;
            }
            magic_close(magic_);
        }
        produce(data);
    }

    void handleInputComplete() {
        setOutputComplete();
    }

    virtual ~BodyRecognitionPlugin() { }

private:
    Transaction &transaction_;
    int recognized_ = 0;
};

class PeekPlugin : public TransactionPlugin {
public:
    PeekPlugin(Transaction &transaction, Logger &logger) : TransactionPlugin(transaction), logger_(logger) {
        registerHook(HOOK_SEND_REQUEST_HEADERS);
        registerHook(HOOK_SEND_RESPONSE_HEADERS);
    }

    void handleSendRequestHeaders(Transaction &transaction) {
        logger_.logInfo("## Request (%lx)",
                reinterpret_cast<unsigned long>(transaction.getAtsHandle()));
        printRequestDiff_(transaction);
        logger_.logInfo("##\n");
        transaction.resume();
    }

    void handleSendResponseHeaders(Transaction &transaction) {
        logger_.logInfo("## Response (%lx)",
                reinterpret_cast<unsigned long>(transaction.getAtsHandle()));
        printResponseDiff_(transaction);
        shared_ptr<PeekContext> ctx = 
            std::dynamic_pointer_cast<PeekContext>(
                transaction.getContextValue("PeekContext"));
        logger_.logInfo("# Body: %s", ctx->body_description.c_str());
        logger_.logInfo("##\n");
        transaction.resume();
    }

private:
    Logger &logger_;

    void printResponseDiff_(Transaction &transaction) {
        Response &sres = transaction.getServerResponse();
        Response &cres = transaction.getClientResponse();

        // Version
        printWithOldValue_("# Version",
            HTTP_VERSION_STRINGS[sres.getVersion()],
            HTTP_VERSION_STRINGS[cres.getVersion()]);

        // Status code
        string sstatus = std::to_string(sres.getStatusCode());
        string cstatus = std::to_string(cres.getStatusCode());
        printWithOldValue_("# Status", sstatus, cstatus);

        // Reason
        printWithOldValue_("# Reason",
            sres.getReasonPhrase(),
            cres.getReasonPhrase());

        // Headers
        printHeaderDiff_(sres.getHeaders(), cres.getHeaders());
    }

    void printRequestDiff_(Transaction &transaction) {
        ClientRequest &creq = transaction.getClientRequest();
        Request &sreq = transaction.getServerRequest();

        // Method
        printWithOldValue_("# Method",
            HTTP_METHOD_STRINGS[creq.getMethod()],
            HTTP_METHOD_STRINGS[sreq.getMethod()]);

        // URL
        printWithOldValue_("# URL",
            creq.getPristineUrl().getUrlString(),
            sreq.getUrl().getUrlString());

        // Version
        printWithOldValue_("# Version",
            HTTP_VERSION_STRINGS[creq.getVersion()],
            HTTP_VERSION_STRINGS[sreq.getVersion()]);

        // Headers
        printHeaderDiff_(creq.getHeaders(), sreq.getHeaders());
    }

    void printHeaderDiff_(Headers &before, Headers &after) {
        string header_name;
        for (Headers::iterator before_ite = before.begin();
            before_ite != before.end();
            ++before_ite) {
            header_name = (*before_ite).name().str();
            Headers::iterator found_ite = after.find(header_name);
            if (found_ite == after.end()) {
                logger_.logInfo("D %s: %s",
                        header_name.c_str(),
                        (*before_ite).values().c_str());
            } else {
                if ((*before_ite).values() == (*found_ite).values()) {
                    logger_.logInfo("= %s: %s",
                            header_name.c_str(),
                            (*found_ite).values().c_str());
                } else {
                    logger_.logInfo("M %s: %s (was %s)",
                            header_name.c_str(),
                            (*found_ite).values().c_str(),
                            (*before_ite).values().c_str());
                }
            }

        }
        for (Headers::iterator after_ite = after.begin();
            after_ite != after.end();
            ++after_ite) {
            header_name = ((*after_ite).name().str());
            Headers::iterator ite = before.find(header_name);
            if (ite == before.end()) {
                logger_.logInfo("A %s: %s",
                        header_name.c_str(),
                        (*after_ite).values().c_str());
            }
        }
    }

    void printWithOldValue_(const string header, const string &before, const string &after) {
        if (before == after) {
            logger_.logInfo("%s: %s",
                header.c_str(),
                after.c_str());
        } else {
            logger_.logInfo("%s: %s (was %s)",
                header.c_str(),
                after.c_str(),
                before.c_str());
        }
    }
};

class GlobalHookPlugin : public GlobalPlugin {
public:
    GlobalHookPlugin() {
        logger_.init("peek.log");
        registerHook(HOOK_READ_REQUEST_HEADERS_PRE_REMAP);
    }

    virtual void handleReadRequestHeadersPreRemap(Transaction &transaction) {
        shared_ptr<PeekContext> ctx = shared_ptr<PeekContext>(new PeekContext());
        transaction.setContextValue("PeekContext", ctx);
        transaction.addPlugin(new PeekPlugin(transaction, logger_));
        transaction.addPlugin(new BodyRecognitionPlugin(transaction,
            TransformationPlugin::RESPONSE_TRANSFORMATION));
        transaction.resume();
    }
private:
    Logger logger_;
};

void TSPluginInit(int argc ATSCPPAPI_UNUSED, const char *argv[] ATSCPPAPI_UNUSED) {
    new GlobalHookPlugin();
}
