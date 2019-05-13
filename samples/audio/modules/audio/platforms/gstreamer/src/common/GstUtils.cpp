/*
 * Copyright 2017-2019 Amazon.com, Inc. or its affiliates. All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License").
 * You may not use this file except in compliance with the License.
 * A copy of the License is located at
 *
 *     http://aws.amazon.com/apache2.0/
 *
 * or in the "license" file accompanying this file. This file is distributed
 * on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either
 * express or implied. See the License for the specific language governing
 * permissions and limitations under the License.
 */

#include "GstUtils.h"

namespace aace {
namespace audio {

bool GstUtils::initializeGStreamer()
{
	GError *err = NULL;
	bool result;
	int argc = 0;
	char **argv = {};
	if (!(result = gst_init_check(&argc, &argv, &err)))
		g_warning("ERROR: %s\n", err->message);
	if (err)
		g_error_free(err);
	return result;
}

GstElement *GstUtils::createElement(
	GstElement *bin, const std::string &factory, const std::string &name)
{
	g_debug("createElement: factory=%s, name=%s\n", factory.c_str(), name.c_str());
	GstElement *element = gst_element_factory_make(factory.c_str(), name.c_str());
	if (!element) {
		g_warning("Unable to create: factory=%s, name=%s\n", factory.c_str(), name.c_str());
		return NULL;
	}
	
	if (!gst_bin_add(GST_BIN(bin), element)) {
		g_warning("Unable to add: name=%s\n",  name.c_str());
		gst_object_unref(element);
		return NULL;
	}

	return element;
}

}
}