/*
 * Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
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

#ifndef AACE_MOBILE_BRIDGE_MOBILE_BRIDGE_H
#define AACE_MOBILE_BRIDGE_MOBILE_BRIDGE_H

#include <memory>
#include <string>
#include <vector>

#include "AACE/Core/PlatformInterface.h"
#include "MobileBridgeEngineInterface.h"
#include "Transport.h"

/** @file */

namespace aace {
namespace mobileBridge {

class MobileBridge : public aace::core::PlatformInterface {
protected:
    MobileBridge() = default;

public:
    class Connection {
    public:
        virtual ~Connection() = default;

        /**
         * Reads up to len bytes of data from the connection into a buffer. This method
         * blocks until input data is available, end of file is detected, or an error occurs.
         *
         * @param buf the buffer into which the data is read.
         * @param off the start offset in array b at which the data is written.
         * @param len the maximum number of bytes to read.
         * @return the total number of bytes read into the buffer, or 0 if there is no more data because the end of the stream has been reached.
         */
        virtual size_t read(uint8_t* buf, size_t off, size_t len) = 0;

        /**
         * Writes len bytes from the specified byte array starting at offset off to this connection.
         *
         * @param buf the data.
         * @param off the start offset in the data.
         * @param len the number of bytes to write.
         */
        virtual void write(const uint8_t* buf, size_t off, size_t len) = 0;

        /**
         * Closes this connection and releases any system resources associated with it.
         */
        virtual void close() = 0;
    };

    ~MobileBridge() override;

    /**
     * Start the mobile bridge.
     *
     * @parm tunFd the file descriptor obtained from system TUN interface.
     */
    bool start(int tunFd);

    /**
     * Stop the mobile bridge.
     */
    bool stop();

    /**
     * Get list of transports.
     */
    virtual std::vector<std::shared_ptr<Transport>> getTransports() = 0;

    /**
     * Connect to the transport.
     */
    virtual std::shared_ptr<Connection> connect(const std::string& transportId) = 0;

    /**
     * Disconnect from the transport.
     */
    virtual void disconnect(const std::string& transportId) = 0;

    /**
     * Notify about the active transport.
     */
    virtual void onActiveTransportChange(const std::string& transportId, const std::string& transportState) = 0;

    /**
     * Notify a device has finished handshaking for some transport. Mobile Bridge engine may
     * request authorization multiple times for the same device since multiple transports are
     * allowed.
     *
     * @param transportId the transport identifier
     * @param deviceToken the string token to identify the device.
     * @param friendlyName device's friendly name.
     */
    virtual void onDeviceHandshaked(
        const std::string& transportId,
        const std::string& deviceToken,
        const std::string& friendlyName) = 0;

    /**
     * Authorize or unauthorized all transports of the specified device.
     *
     * @param deviceToken the string token to identify the device.
     * @param authorized indicates whether the device is authorized.
     */
    void authorizeDevice(const std::string& deviceToken, bool authorized);

    /**
     * Receive an info from a connected device.
     *
     * @param deviceToken the string token to identify the device.
     * @param infoId the info id to identify a unique info.
     * @param info the info string.
     */
    virtual void onInfo(const std::string& deviceToken, uint32_t infoId, const std::string& info) = 0;

    /**
     * Send an info to the specified device.
     *
     * @param deviceToken the string token to identify the device.
     * @param infoId the info id to identify a unique info.
     * @param info the info string.
     */
    void sendInfo(const std::string& deviceToken, uint32_t infoId, const std::string& info);

    /**
     * Protect a socket from VPN connections.
     *
     * @param sock the socket to protect.
     * @return true on success.
     */
    virtual bool protectSocket(int sock) = 0;

    /**
     * @internal
     * Sets the Engine interface delegate.
     *
     * Should *never* be called by the platform implementation.
     */
    void setEngineInterface(std::shared_ptr<MobileBridgeEngineInterface> mobileBridgeEngineInterface);

private:
    std::weak_ptr<MobileBridgeEngineInterface> m_mobileBridgeEngineInterface;
};

}  // namespace mobileBridge
}  // namespace aace

#endif  // AACE_MOBILE_BRIDGE_MOBILE_BRIDGE_H
