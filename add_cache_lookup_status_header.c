/** @add_cache_lookup_status_header.c

  Add cache lookup status to response header

  @section license License

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

/* add_cache_lookup_status_header.c: a plugin that adds cache lookup status to
 *               client response headers.
 *
 *
 *   Usage:
 *     add_cache_lookup_status_header.so [header_name]
 *
 *          header_name is the name of header to be added to
 *          the client response.
 *          the default value of header_name is X-Cache.
 */

#include "ts/ts.h"

#define PLUGIN_NAME "add_cache_lookup_status_header"
#define DEFAULT_HEADER_NAME "X-Cache"

static void
add_cache_lookup_status_header(TSHttpTxn txnp, TSCont contp)
{
  TSMBuffer res_bufp;
  TSMLoc res_loc;
  TSMLoc field_loc;
  int lookup_status;
  const char *header_name;
  const char *header_value;
  int retval;

  retval = TSHttpTxnCacheLookupStatusGet(txnp, &lookup_status);
  if (retval != TS_SUCCESS) {
    TSError("[%s] Unable to get cache lookup status", PLUGIN_NAME);
    goto error;
  }
  switch (lookup_status) {
  case TS_CACHE_LOOKUP_MISS:
    header_value = "MISS";
  	break;
  case TS_CACHE_LOOKUP_HIT_STALE:
    header_value = "HIT_STALE";
  	break;
  case TS_CACHE_LOOKUP_HIT_FRESH:
    header_value = "HIT_FRESH";
  	break;
  case TS_CACHE_LOOKUP_SKIPPED:
    header_value = "SKIPPED";
  	break;
  default:
    TSError("[%s] Unexpected lookup_status header_value", PLUGIN_NAME);
    goto error;
  }

  retval = TSHttpTxnClientRespGet(txnp, &res_bufp, &res_loc);
  if (retval != TS_SUCCESS) {
    TSError("[%s] Unable to retrieve server response", PLUGIN_NAME);
    goto done;
  }

  header_name = (const char *)TSContDataGet(contp);
  field_loc = TSMimeHdrFieldFind(res_bufp, res_loc, header_name, -1);
  if (field_loc == TS_NULL_MLOC) {
    retval = TSMimeHdrFieldCreate(res_bufp, res_loc, &field_loc);
    if (retval != TS_SUCCESS) {
      TSError("[%s] Unable to create new field", PLUGIN_NAME);
      goto error;
    }

    retval = TSMimeHdrFieldNameSet(res_bufp, res_loc, field_loc, header_name, -1);
    if (retval != TS_SUCCESS) {
      TSError("[%s] Unable to create new field", PLUGIN_NAME);
      goto error;
    }

    retval = TSMimeHdrFieldAppend(res_bufp, res_loc, field_loc);
    if (retval != TS_SUCCESS) {
      TSError("[%s] Unable to append new field", PLUGIN_NAME);
      goto error;
    }
  }

  retval = TSMimeHdrFieldValueStringSet(res_bufp, res_loc, field_loc, -1, header_value, -1);
  if (retval != TS_SUCCESS) {
    TSError("[%s] Unable to set field header_value", PLUGIN_NAME);
    goto error;
  }

  TSDebug(PLUGIN_NAME, "set header name=\"%s\" value=\"%s\"", header_name, header_value);

error:
  if (field_loc != TS_NULL_MLOC) {
    TSHandleMLocRelease(res_bufp, res_loc, field_loc);
  }
  TSHandleMLocRelease(res_bufp, TS_NULL_MLOC, res_loc);

done:
  TSHttpTxnReenable(txnp, TS_EVENT_HTTP_CONTINUE);
}

static int
global_hook_handler(TSCont contp, TSEvent event, void *edata)
{
  TSHttpTxn txnp = (TSHttpTxn)edata;

  switch (event) {
  case TS_EVENT_HTTP_SEND_RESPONSE_HDR:
    add_cache_lookup_status_header(txnp, contp);
    break;
  default:
    TSError("[%s] unknown event for this plugin: event=%d", PLUGIN_NAME, event);
    break;
  }
  return 0;
}

void
TSPluginInit(int argc, const char *argv[])
{
  TSPluginRegistrationInfo info;
  const char *header_name;
  TSCont contp;

  info.plugin_name = "add_cache_lookup_status_header";
  info.vendor_name = "Hiroaki Nakamura";
  info.support_email = "hnakamur@gmail.com";

  if (TSPluginRegister(&info) != TS_SUCCESS) {
    TSError("[%s] Plugin registration failed.", PLUGIN_NAME);
    return;
  }

  if (argc > 2) {
    TSError("[%s] Usage: %s [header_name]", PLUGIN_NAME, argv[0]);
    return;
  }

  header_name = TSstrdup(argc > 1 ? argv[1] : DEFAULT_HEADER_NAME);

  contp = TSContCreate(global_hook_handler, TSMutexCreate());
  TSContDataSet(contp, (void *)header_name);
  TSHttpHookAdd(TS_HTTP_SEND_RESPONSE_HDR_HOOK, contp);
}
