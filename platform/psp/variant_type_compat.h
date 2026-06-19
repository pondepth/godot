#pragma once
#include "core/variant/type_info.h"
#ifdef PSP_ENABLED
template <>
struct GetTypeInfo<Math::int_alt_t> {
static const Variant::Type VARIANT_TYPE = Variant::INT;
static const GodotTypeInfo::Metadata METADATA = GodotTypeInfo::METADATA_INT_IS_INT32;
static inline PropertyInfo get_class_info() { return PropertyInfo(VARIANT_TYPE, String()); }
};
template <>
struct GetTypeInfo<Math::uint_alt_t> {
static const Variant::Type VARIANT_TYPE = Variant::INT;
static const GodotTypeInfo::Metadata METADATA = GodotTypeInfo::METADATA_INT_IS_UINT32;
static inline PropertyInfo get_class_info() { return PropertyInfo(VARIANT_TYPE, String()); }
};
#endif
