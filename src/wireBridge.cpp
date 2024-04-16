#include "wireBridge.hpp"

std::vector<std::vector<BoolStorageAccessor>> wireBridge(DataBank<1>& db, const WireBridgeData& ports)
{
    auto [storage] = db.lendStorage();
    std::vector<std::vector<BoolStorageAccessor>> result;
    result.reserve(ports.size());
    for (const auto& port : ports) {
        std::vector<BoolStorageAccessor> accessors;
        accessors.reserve(port.size());
        size_t offset = 0;
        for (const auto& size : port) {
            accessors.emplace_back(BoolStorageAccessor(offset, size, storage));
            offset += size;
        }
        if (offset > STORAGE_SIZE) {
            throw std::runtime_error("Port size exceeds STORAGE_SIZE");
        }
        result.emplace_back(accessors);
    }
    return result;
}
