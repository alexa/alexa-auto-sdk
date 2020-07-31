// #include <AVSCommon/Utils/Logger/LogEntry.h>
// #include "AVSCommon/Utils/Metrics/MetricEvent.h"
// #include "AVSCommon/Utils/Metrics.h"

#include "AACE/Engine/Metrics/MetricEvent.h"
#include "AACE/Engine/Core/EngineMacros.h"

namespace aace {
namespace engine {
namespace metrics {

static const std::string TAG("MetricEvent");

/// Default number of samples for metric.
const std::string METRIC_NUM_SAMPLES_DEFAULT = "1";

MetricEvent::MetricEvent(const std::string& program, const std::string& source) :
        MetricEvent(program, source, MetricPriority::NR) {
}

MetricEvent::MetricEvent(const std::string& program, const std::string& source, MetricPriority priority) :
        m_program{program}, m_source(source), m_metricLog{""}, m_priority{priority} {
    m_metricLog.append(m_program).append(":").append(m_source);
}

void MetricEvent::addTimer(const std::string& name, double value) {
    addDataToLog(name, std::to_string(value), MetricDataType::TI, METRIC_NUM_SAMPLES_DEFAULT);
}

void MetricEvent::addString(const std::string& name, const std::string& value) {
    addDataToLog(name, value, MetricDataType::DV, METRIC_NUM_SAMPLES_DEFAULT);
}

void MetricEvent::addCounter(const std::string& name, int value) {
    addDataToLog(name, std::to_string(value), MetricDataType::CT, METRIC_NUM_SAMPLES_DEFAULT);
}

void MetricEvent::record() {
    std::string priorityStr = priorityToString(m_priority);
    m_metricLog.append(":").append(priorityStr);
    AACE_METRIC(LX(TAG, m_metricLog));
}

void MetricEvent::addDataToLog(std::string name, std::string value, MetricDataType type, std::string sampleCount) {
    m_metricLog.append(":").append(name).append("=").append(value).append(";");
    m_metricLog.append(dataTypeToString(type)).append(";").append(sampleCount).append(",");
}

std::string MetricEvent::priorityToString(MetricPriority priority) {
    switch (priority) {
        case MetricPriority::NR:
            return "NR";
        case MetricPriority::HI:
            return "HI";
    }
    return "";
}

std::string MetricEvent::dataTypeToString(MetricDataType type) {
    switch (type) {
        case MetricDataType::TI:
            return "TI";
        case MetricDataType::DV:
            return "DV";
        case MetricDataType::CT:
            return "CT";
    }
    return "";
}

}  // namespace metrics
}  // namespace engine
}  // namespace aace