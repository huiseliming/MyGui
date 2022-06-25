#include "Reflect/Type.h"

#define GET_BUILTIN_TYPE_FUNCTION_IMPL(BuiltinType, NameString)        \
	template<> Type* GetStaticType<BuiltinType>()                    \
	{                                                                  \
		static TSimpleStaticType<BuiltinType> NameString##Type(#NameString);\
		return &NameString##Type;                                      \
	}

namespace Reflect
{
	GET_BUILTIN_TYPE_FUNCTION_IMPL(bool, Bool)
	GET_BUILTIN_TYPE_FUNCTION_IMPL(int8_t, SInt8)
	GET_BUILTIN_TYPE_FUNCTION_IMPL(int16_t, SInt16)
	GET_BUILTIN_TYPE_FUNCTION_IMPL(int32_t, SInt32)
	GET_BUILTIN_TYPE_FUNCTION_IMPL(int64_t, SInt64)
	GET_BUILTIN_TYPE_FUNCTION_IMPL(uint8_t, UInt8)
	GET_BUILTIN_TYPE_FUNCTION_IMPL(uint16_t, UInt16)
	GET_BUILTIN_TYPE_FUNCTION_IMPL(uint32_t, UInt32)
	GET_BUILTIN_TYPE_FUNCTION_IMPL(uint64_t, UInt64)
	GET_BUILTIN_TYPE_FUNCTION_IMPL(float, Float)
	GET_BUILTIN_TYPE_FUNCTION_IMPL(double, Double)
	GET_BUILTIN_TYPE_FUNCTION_IMPL(std::string, String)

	template<> Type* GetStaticType<bool>();
	template<> Type* GetStaticType<int8_t>();
	template<> Type* GetStaticType<int16_t>();
	template<> Type* GetStaticType<int32_t>();
	template<> Type* GetStaticType<int64_t>();
	template<> Type* GetStaticType<uint8_t>();
	template<> Type* GetStaticType<uint16_t>();
	template<> Type* GetStaticType<uint32_t>();
	template<> Type* GetStaticType<uint64_t>();
	template<> Type* GetStaticType<float>();
	template<> Type* GetStaticType<double>();
	template<> Type* GetStaticType<std::string>();

	std::unordered_map<std::type_index, Type*>& GetTypeIndexMap()
	{
		static std::unordered_map<std::type_index, Type*> type_index_map;
		return type_index_map;
	}
}