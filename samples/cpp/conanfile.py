import os, json, logging
from conans import ConanFile, tools, CMake


class SampleApp(ConanFile):

    name = "aac-sampleapp"
    generators = ["cmake"]
    settings = "os", "compiler", "build_type", "arch"
    python_requires = "aac-sdk-tools/1.0"
    requires = [
        "nlohmann_json/3.8.0#d99228f0d54216a1cd1c1153873377c5",
        "gsl-lite/0.37.0#1421c56db9091e68c9d32a2e721a5616",
    ]
    exports_sources = "*"

    options = {
        "aac_modules": "ANY",
        "extra_modules": "ANY",
        "with_aasb": [True, False],
        "with_unit_tests": [True, False],
        "with_sensitive_logs": [True, False],
        "with_metrics": [True, False],
        "with_3pva":[True, False],
    }

    default_options = {
        "aac_modules": "core,alexa,cbl,system-audio",
        "extra_modules": None,
        "with_aasb": True,
        "with_unit_tests": False,
        "with_sensitive_logs": False,
        "with_metrics": True,
        "with_3pva": False,
    }

    def set_version(self):
        if self.version == None:
            self.version = "dev"

    def requirements(self):
        module_list = set(str(self.options.aac_modules).split(","))
        if self.options.extra_modules:
            module_list.update(str(self.options.extra_modules).split(","))
        for next in module_list:
            req = next if next.startswith("aac-module-") else f"aac-module-{next.strip().casefold()}"
            self.requires(f"{req}/{self.version}")
            # configure options
            self.options[req].with_aasb = self.options.with_aasb
            self.options[req].with_unit_tests = self.options.with_unit_tests
            self.options[req].with_sensitive_logs = self.options.with_sensitive_logs
            self.options[req].with_metrics = self.options.with_metrics

    def build(self):
        cmake = self._configure_cmake()
        cmake.build()

    def get_config_opt(self, env_name, def_val=None):
        return def_val if def_val else os.getenv(env_name)

    def replace_token(self, file, token, def_val=None):
        # use the default value if provided, otherwise get the value from
        # the specified environment variable
        value = self.get_config_opt(token, def_val)
        if value:
            tools.replace_in_file(file, "${%s}" % token, str(value))
        elif not self.in_local_cache:
            logging.warning("Configuration value not defined: ${%s}" % token)

    @property
    def def_certs_path(self):
        certs_path = self.get_config_opt("CERTS_PATH")
        return certs_path if certs_path or self.in_local_cache else os.path.join(self.install_folder, "certs")

    @property
    def def_data_path(self):
        data_path = self.get_config_opt("DATA_PATH")
        return data_path if data_path or self.in_local_cache else os.path.join(self.install_folder, "data")

    def install_shared_data(self, src_path, dst_path):
        aac_sdk_tools_path = self.python_requires["aac-sdk-tools"].path
        # copy the shared resources
        self.copy(
            "*", src=os.path.join(src_path, "assets/config"), dst=os.path.join(dst_path, "config"), keep_path=True
        )
        self.copy("*", src=os.path.join(src_path, "assets/menu"), dst=os.path.join(dst_path, "menu"), keep_path=True)
        self.copy(
            "*", src=os.path.join(src_path, "assets/inputs"), dst=os.path.join(dst_path, "inputs"), keep_path=True
        )
        self.copy(
            "*",
            src=os.path.join(src_path, "assets/sampledata"),
            dst=os.path.join(dst_path, "sampledata"),
            keep_path=True,
        )
        self.copy(
            "*", src=os.path.join(aac_sdk_tools_path, "certs"), dst=os.path.join(dst_path, "certs"), keep_path=True
        )
        # replace tokens in the config file with default data from the proejct
        config_file = os.path.join(os.path.join(dst_path, "config"), "config.json")
        # certs and data config
        self.replace_token(config_file, "CERTS_PATH", self.def_certs_path)
        self.replace_token(config_file, "DATA_PATH", self.def_data_path)
        # alexa client info config
        self.replace_token(config_file, "CLIENT_ID")
        self.replace_token(config_file, "PRODUCT_ID")
        self.replace_token(config_file, "AMAZON_ID")
        # device config
        self.replace_token(config_file, "DEVICE_MANUFACTURER")
        self.replace_token(config_file, "DEVICE_MODEL")
        self.replace_token(config_file, "DEVICE_PLATFORM")
        self.replace_token(config_file, "DEVICE_OS_VERSION")
        self.replace_token(config_file, "DEVICE_HARDWARE_ARCH")
        self.replace_token(config_file, "DEVICE_SERIAL_NUMBER")
        # app info config
        self.replace_token(config_file, "APP_VERSION")

        # vehicle info config
        self.replace_token(config_file, "VEHICLE_MAKE")
        self.replace_token(config_file, "VEHICLE_MODEL")
        self.replace_token(config_file, "VEHICLE_YEAR")
        self.replace_token(config_file, "VEHICLE_TRIM")
        self.replace_token(config_file, "VEHICLE_MICROPHONE")
        self.replace_token(config_file, "VEHICLE_COUNTRY")
        self.replace_token(config_file, "VEHICLE_IDENTIFIER")
        self.replace_token(config_file, "VEHICLE_ENGINE_TYPE")
        self.replace_token(config_file, "VEHICLE_RSE_EMBEDDED_FIRETVS")

        # metric info
        self.replace_token(config_file, "METRIC_TAG")
        
    def _configure_cmake(self):
        cmake = CMake(self)
        defs = {}
        if self.options.with_3pva:
            module_list = set(str(self.options.aac_modules).split(","))
            if "aac-module-amazonlite" in module_list:
                # right Pryonlite Package-suffix required for Pryonlite VAD capability
                if self.deps_user_info["pryon-lite"].package_suffix == "auto-sdk-multi-ww":
                    defs["AAC_VAD_ENABLE"] = "ON"
                else:
                    raise Exception("Pryonlite auto-sdk-multi-ww package is missing")
            else:
                raise Exception("Pryonlite VAD capability requires amazonlite module")
        cmake.configure(defs=defs)
        return cmake

    def imports(self):
        if not self.in_local_cache:
            # install dependencies
            self.copy("*.dylib*", dst="lib", keep_path=False)
            self.copy("*.so*", dst="lib", keep_path=False)
            self.copy("*.dll*", dst="bin", keep_path=False)
            # install assets
            self.install_shared_data(self.recipe_folder, self.install_folder)
            # create the data folder
            tools.mkdir(os.path.join(self.install_folder, "data"))

    def package(self):
        self.copy("*", src="bin", dst="bin", keep_path=False)
        # isntall the shared data into the package
        self.install_shared_data(self.build_folder, os.path.join(self.package_folder, "share"))

    def package_info(self):
        self.user_info.aac_export_bins = json.dumps([{"pattern": "*", "src": os.path.abspath("bin")}])
        self.user_info.aac_export_shared = json.dumps(
            [
                {
                    "pattern": "*",
                    "src": os.path.abspath("share"),
                    "dst": "sampleapp",
                }
            ]
        )
