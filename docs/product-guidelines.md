# Product Requirements and Guidelines

Amazon provides requirements you must follow when integrating Alexa Auto SDK into your vehicle. To verify your integration meets the requirements, passing Amazon's automotive certification process is a prerequisite to launch your Alexa in-vehicle experience. Refer to the guidelines below when designing and developing your integration, and contact your Amazon Solutions Architect (SA) or Partner Manager to guide you through the certification process.

## Customer experience requirements

The [Alexa Automotive customer experience requirements](https://github.com/alexa/alexa-auto-sdk/blob/master/Alexa_Embedded_CX_Requirements_v12.0.xlsx) define product requirements your integration must meet in order to provide a customer experience that passes Amazon's certification process.

## Product and UX guidelines

The [Alexa Automotive design guidelines](https://developer.amazon.com/en-US/docs/alexa/alexa-auto/about-this-guide.html) define principles, user interface patterns, and multi-modal best practices for Alexa automotive experiences.

## Security best practices

All Alexa products are required to follow the [Security Best Practices for Alexa](https://developer.amazon.com/en-US/docs/alexa/alexa-voice-service/avs-security-reqs.html). When building an Alexa experience using the Auto SDK, additionally adhere to the following security principles:

* Protect configuration files for Auto SDK from tampering and inspection.
* Protect configuration parameters, such as those found in Auto SDK configuration files, from tampering and inspection, including but not limited to the following: SQLite database files, Unix Domain Sockets, wake word models, and metrics sink files.
* Protect components used for the Local Voice Control (LVC) extension, including associated LVC language model packages (Linux) and APKs (Android), from tampering and inspection, including but not limited to the following: Unix Domain Sockets, model directories, skill and service executables, prompts and assets JSON files, and all files configuring these components. 
* Your implementation of Auto SDK interfaces must not retain locks, crash, hang, or throw uncaught exceptions.
* Use exploit mitigation flags and memory randomization techniques when you compile your source code to prevent vulnerabilities from exploiting buffer overflows and memory corruptions. 