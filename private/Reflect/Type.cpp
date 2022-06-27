#include "Reflect/Type.h"

#define GET_STATIC_TYPE_FUNCTION_IMPL(BuiltinType, NameString)              \
	template<> Type* GetStaticType<BuiltinType>()                           \
	{                                                                       \
		static TSimpleStaticType<BuiltinType> NameString##Type(#NameString);\
		return &NameString##Type;                                           \
	}

namespace Reflect
{
	GET_STATIC_TYPE_FUNCTION_IMPL(bool       , Bool)
	GET_STATIC_TYPE_FUNCTION_IMPL(float      , Float)
	GET_STATIC_TYPE_FUNCTION_IMPL(double     , Double)
	GET_STATIC_TYPE_FUNCTION_IMPL(int8_t     , SInt8)
	GET_STATIC_TYPE_FUNCTION_IMPL(int16_t    , SInt16)
	GET_STATIC_TYPE_FUNCTION_IMPL(int32_t    , SInt32)
	GET_STATIC_TYPE_FUNCTION_IMPL(int64_t    , SInt64)
	GET_STATIC_TYPE_FUNCTION_IMPL(uint8_t    , UInt8)
	GET_STATIC_TYPE_FUNCTION_IMPL(uint16_t   , UInt16)
	GET_STATIC_TYPE_FUNCTION_IMPL(uint32_t   , UInt32)
	GET_STATIC_TYPE_FUNCTION_IMPL(uint64_t   , UInt64)
	GET_STATIC_TYPE_FUNCTION_IMPL(std::string, String)

	GET_STATIC_TYPE_FUNCTION_IMPL(std::vector<std::any>, Vector)

	using PtrMap             = std::map<void*, std::any>;
	using Int64Map           = std::map<int64_t, std::any>;
	using StringMap          = std::map<std::string, std::any>;
	using PtrUnorderedMap    = std::unordered_map<void*, std::any>;
	using Int64UnorderedMap  = std::unordered_map<int64_t, std::any>;
	using StringUnorderedMap = std::unordered_map<std::string, std::any>;

	GET_STATIC_TYPE_FUNCTION_IMPL(PtrMap            , PtrMap)
	GET_STATIC_TYPE_FUNCTION_IMPL(Int64Map          , Int64Map)
	GET_STATIC_TYPE_FUNCTION_IMPL(StringMap         , StringMap)
	GET_STATIC_TYPE_FUNCTION_IMPL(PtrUnorderedMap   , PtrUnorderedMap)
	GET_STATIC_TYPE_FUNCTION_IMPL(Int64UnorderedMap , Int64UnorderedMap)
	GET_STATIC_TYPE_FUNCTION_IMPL(StringUnorderedMap, StringUnorderedMap)

	std::unordered_map<std::type_index, Type*>& GetTypeIndexMap()
	{
		static std::unordered_map<std::type_index, Type*> static_type_index_map;
		return static_type_index_map;
	}

	Type* GetType(const std::type_info& type_info)
	{
		std::unordered_map<std::type_index, Type*>& type_index_map_ref = GetTypeIndexMap();
		auto type_index_map_iterator = type_index_map_ref.find(type_info);
		if (type_index_map_ref.end() != type_index_map_iterator)
			return type_index_map_iterator->second;
		return nullptr;
	}

	// 
	static struct SimpleStaticTypeAutoInitializer
	{
		SimpleStaticTypeAutoInitializer() {
			GetType<bool       >();
			GetType<float      >();
			GetType<double     >();
			GetType<int8_t     >();
			GetType<int16_t	   >();
			GetType<int32_t	   >();
			GetType<int64_t	   >();
			GetType<uint8_t	   >();
			GetType<uint16_t   >();
			GetType<uint32_t   >();
			GetType<uint64_t   >();
			GetType<std::string>();
			GetType<std::vector<std::any>>();

			GetType<std::map<void*, std::any>>();
			GetType<std::map<int64_t, std::any>>();
			GetType<std::map<std::string, std::any>>();

			GetType<std::unordered_map<void*, std::any>>();
			GetType<std::unordered_map<int64_t, std::any>>();
			GetType<std::unordered_map<std::string, std::any>>();
		}
	} SSimpleStaticTypeAutoInitializer;
}