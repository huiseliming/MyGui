#include "Reflect/Type.h"
#include "Reflect/Enum.h"
#include "Reflect/Class.h"

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

	std::unordered_map<std::string, Type*>& GetTypeNameMap()
	{
		static std::unordered_map<std::string, Type*> static_type_name_map;
		return static_type_name_map;
	}
	std::unordered_map<std::string, Type*>& GTypeNameMap = GetTypeNameMap();

	void AddTypeToNameMap(const std::string& type_name, Type* type_ptr)
	{
		std::unordered_map<std::string, Type*>& type_name_map_ref = GetTypeNameMap();
		assert(type_name_map_ref.insert(std::pair(type_name, type_ptr)).second);
	}

	Enum* FindEnum(const std::string& enum_name)
	{
		auto type_name_map_iterator = GTypeNameMap.find(enum_name);
		if (type_name_map_iterator != GTypeNameMap.end())
		{
			if (type_name_map_iterator->second->GetCastTypeFlag() & CTFB_EnumBit)
			{
				return static_cast<Enum*>(type_name_map_iterator->second);
			}
		}
		return nullptr;
	}

	Class* FindClass(const std::string& class_name)
	{
		auto type_name_map_iterator = GTypeNameMap.find(class_name);
		if (type_name_map_iterator != GTypeNameMap.end())
		{
			if (type_name_map_iterator->second->GetCastTypeFlag() & CTFB_ClassBit)
			{
				return static_cast<Class*>(type_name_map_iterator->second);
			}
		}
		return nullptr;
	}

	std::unordered_map<std::type_index, Type*>& GetTypeIndexMap()
	{
		static std::unordered_map<std::type_index, Type*> static_type_index_map;
		return static_type_index_map;
	}
	std::unordered_map<std::type_index, Type*>& GTypeIndexMap = GetTypeIndexMap();

	Type* GetType(const std::type_info& type_info)
	{
		std::unordered_map<std::type_index, Type*>& type_index_map_ref = GetTypeIndexMap();
		auto type_index_map_iterator = type_index_map_ref.find(type_info);
		if (type_index_map_ref.end() != type_index_map_iterator)
			return type_index_map_iterator->second;
		return nullptr;
	}

	std::vector<std::unique_ptr<PointerType>>& GetStaticPointerTypes()
	{
		static std::vector<std::unique_ptr<PointerType>> static_pointer_type_vector;
		return static_pointer_type_vector;
	}

	std::unordered_map<std::type_index, PointerType*>& GetUninitializePointerTypeMap()
	{
		static std::unordered_map<std::type_index, PointerType*> static_uninitialize_pointer_type_map;
		return static_uninitialize_pointer_type_map;
	}

	std::unordered_map<std::type_index, Type*>& GetPointerTypePointToTypeIndexMap()
	{
		static std::unordered_map<std::type_index, Type*> static_pointer_type_point_to_type_index_map;
		return static_pointer_type_point_to_type_index_map;
	}

	bool VerifyStaticTypeInitializationResult()
	{
		auto& uninitialize_pointer_type_map_ref = GetUninitializePointerTypeMap();
		if (!uninitialize_pointer_type_map_ref.empty())
		{
			for (auto& [key, value] : uninitialize_pointer_type_map_ref)
			{
				spdlog::error("uninitialize pointer type {:s}", key.name());
			}
			return false;
		}
		return true;
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