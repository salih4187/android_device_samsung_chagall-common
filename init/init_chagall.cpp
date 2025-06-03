/*
   Copyright (c) 2020, The LineageOS Project. All rights reserved.
   Redistribution and use in source and binary forms, with or without
   modification, are permitted provided that the following conditions are
   met:
    * Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above
      copyright notice, this list of conditions and the following
      disclaimer in the documentation and/or other materials provided
      with the distribution.
    * Neither the name of The Linux Foundation nor the names of its
      contributors may be used to endorse or promote products derived
      from this software without specific prior written permission.
   THIS SOFTWARE IS PROVIDED "AS IS" AND ANY EXPRESS OR IMPLIED
   WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
   MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT
   ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS
   BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
   CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
   SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
   BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
   WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
   OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN
   IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>

#define _REALLY_INCLUDE_SYS__SYSTEM_PROPERTIES_H_
#include <sys/_system_properties.h>
#include <sys/stat.h>
#include <sys/types.h>

#include <android-base/file.h>
#include <android-base/logging.h>
#include <android-base/properties.h>
#include <android-base/strings.h>

#include "vendor_init.h"

using android::base::GetProperty;
using android::base::ReadFileToString;
using android::base::Trim;
using std::string;

// copied from build/tools/releasetools/ota_from_target_files.py
// but with "." at the end and empty entry
std::vector<std::string> ro_props_default_source_order = {
    "",
    "product.",
    "product_services.",
    "odm.",
    "vendor.",
    "system.",
    "system_ext.",
};

void property_override(string prop, string value) {
    auto pi = (prop_info*) __system_property_find(prop.c_str());

    if (pi != nullptr)
        __system_property_update(pi, value.c_str(), value.size());
    else
        __system_property_add(prop.c_str(), prop.size(), value.c_str(), value.size());
}

void set_ro_build_prop(const string &prop, const string &value, bool product = true) {
    string prop_name;

    for (const auto &source : ro_props_default_source_order) {
        if (product)
            prop_name = "ro.product." + source + prop;
        else
            prop_name = "ro." + source + "build." + prop;

        property_override(prop_name.c_str(), value.c_str());
    }
}

void set_build_fingerprint(string name, string device, string build) {
    string build_fingerprint;
    string build_desc;

    std::string bl = GetProperty("ro.bootloader","");
    if(bl.empty()){
        bl = GetProperty("ro.boot.bootloader", "");
    }
    
    LOG(ERROR) << "Found bootloader id " << bl << " setting build properties for " << device << " device" << std::endl;

    build_fingerprint = "samsung/" + name + "/" + device + ":6.0.1/MMB29K/" + build + ":user/release-keys";
    build_desc = name + "--user 6.0.1 MMB29K " + build + " release-keys";

    set_ro_build_prop("fingerprint", build_fingerprint, false);
    set_ro_build_prop("description", build_desc, false);
    property_override("ro.bootimage.build.fingerprint", build_fingerprint);
    
    set_ro_build_prop("name", name);
    set_ro_build_prop("name", name, false);
    set_ro_build_prop("device", device);
    set_ro_build_prop("device", device, false);
}

void vendor_load_properties() {
    string model;

    std::string bootloader = GetProperty("ro.bootloader","");
    if(bootloader.empty()){
        bootloader = GetProperty("ro.boot.bootloader", "");
    }

    /* name , device , build */
	/* samsung/chagallltexx/chagalllte:6.0.1/MMB29K/T805DDS1CQD2:user/release-keys */
	/* ro.build.description=chagallltexx-user 6.0.1 MMB29K T805DDS1CQD2 release-keys */

    if (bootloader.find("T805") == 0) {
        set_build_fingerprint("chagallltexx", "chagalllte", "T805DDS1CQD2");
        model = "SM-T805";
    } else if (bootloader.find("T807") == 0) {
        set_build_fingerprint("chagall3gxx", "chagall3g", "T807JVS1CRH1");
        model = "SM-T807";
    } else if (bootloader.find("T800") == 0) {
        set_build_fingerprint("chagallwifixx", "chagallwifi", "T800DDS1CQD2");
        model = "SM-T800";
    }
    
    set_ro_build_prop("model", model);
    set_ro_build_prop("product", model, false);
    
}

