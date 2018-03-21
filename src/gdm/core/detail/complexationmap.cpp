#include "complexationmap.h"

#include <algorithm>
#include <numeric>
#include <iterator>
#include <cassert>

gdm::detail::ComplexationMap gdm::detail::findAll(const ComplexationMap& obj, ConstituentType constituentType, const std::string& constituentName)
{
    ComplexationMap ret{};

    std::copy_if(obj.cbegin(), obj.cend(),
                 std::inserter(ret, ret.end()),
                 [constituentType, &constituentName](const auto& entry){ return (name(entry.first, constituentType) == constituentName); });

    return ret;
}

gdm::ChargeInterval gdm::detail::chargeSpan(const ComplexationMap& obj, ConstituentType constituentType, const std::string& constituentName)
{
    auto entries = findAll(obj, constituentType, constituentName);

    return std::accumulate(entries.cbegin(), entries.cend(),
                           ChargeInterval{},
                           [constituentType](auto accum, const auto& entry)
                            {
                                return hull(accum, chargeSpan(entry.second, constituentType));
                            });
}

void gdm::detail::eraseAll(ComplexationMap& obj, ConstituentType constituentType, const std::string& constituentName) noexcept
{
    for(auto it = obj.cbegin(); it != obj.cend(); /*nothing*/) { //If-erase loop
        if(name(it->first, constituentType) == constituentName) {
            it = obj.erase(it);
        }
        else ++it;
    }

    assert(findAll(obj, constituentType, constituentName).empty());
}

void gdm::detail::clampAll(ComplexationMap& obj, ConstituentType constituentType, const std::string& constituentName, ChargeInterval interval) noexcept
{
    ComplexationMap newObj = obj;

    for(auto& entry : obj) {
        auto it = newObj.find(entry.first);
        auto& complexation = it->second;
        if(name(it->first, constituentType) == constituentName) clamp(complexation, constituentType, interval);
        if (it->second.size() == 0) newObj.erase(it->first);
    }

    obj = std::move(newObj);

    assert(contains(interval, chargeSpan(obj, constituentType, constituentName)));
}


void gdm::detail::rename(ComplexationMap& obj, ConstituentType constituentType, const std::string& constituentName, const std::string& newName)
{
    auto initialSize = obj.size();

    //Get a copy of the entries that need to be renamed
    auto toBeRenamed = findAll(obj, constituentType, constituentName);

    //Erase them from the container
    eraseAll(obj, constituentType, constituentName);

    //Re-insert them into the container after being renamed
    std::transform(toBeRenamed.cbegin(), toBeRenamed.cend(),
                   std::inserter(obj, obj.end()),
                   [&constituentType, &constituentName, &newName](const auto& entry)
                    {
                        auto key = entry.first;
                        const auto& value = entry.second;

                        assert(name(key, constituentType) == constituentName);
                        setName(key, constituentType, newName);

                        return std::make_pair(key, value);
                    });

    assert(findAll(obj, constituentType, constituentName).empty());
    assert(obj.size() == initialSize);
}
