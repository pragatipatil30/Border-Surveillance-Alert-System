#include "api/services/dsa_runtime.hpp"

namespace border_api::services {

DsaRuntime& DsaRuntime::instance()
{
    static DsaRuntime runtime;
    return runtime;
}

} // namespace border_api::services
