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

#include <AACE/Engine/Core/EngineMacros.h>

#include "GstUtils.h"

namespace aace {
namespace audio {

static const std::string TAG("aace.gstreamer.GstUtils");

void GstUtils::initializeGStreamer()
{
	int argc = 0;
	char **argv = {};
	gst_init(&argc, &argv);
}

GstElement *GstUtils::createElement(
	GstElement *bin, const std::string &factory, const std::string &name)
{
	AACE_DEBUG(LX(TAG, "createElement")
		.d("factory", factory)
		.d("name", name));

	GstElement *element = gst_element_factory_make(factory.c_str(), name.c_str());
	if (!element) {
		AACE_ERROR(LX(TAG, "createElement")
			.m("Unable to create")
			.d("factory", factory)
			.d("name", name));
		return NULL;
	}
	
	if (!gst_bin_add(GST_BIN(bin), element)) {
		AACE_ERROR(LX(TAG, "createElement")
			.m("Unable to add")
			.d("name", name));
		gst_object_unref(element);
		return NULL;
	}

	return element;
}

}
}