#include "Core/Type.h"
#include "Core/Enum.h"
#include "Core/Class.h"

#define GET_STATIC_TYPE_FUNCTION_IMPL(BuiltinType, NameString)              \
	template<> Type* GetStaticType<BuiltinType>()                           \
	{                                                                       \
		static TSimpleStaticType<BuiltinType> NameString##Type(#NameString);\
		return &NameString##Type;                                           \
	}

namespace Core
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

	std::unordered_map<std::string, Class*>& GetClassNameMap()
	{
		static std::unordered_map<std::string, Class*> static_class_name_map;
		return static_class_name_map;
	}

	std::unordered_map<std::string, Enum*>& GetEnumNameMap()
	{
		static std::unordered_map<std::string, Enum*> static_enum_name_map;
		return static_enum_name_map;
	}

	std::unordered_map<std::string, Class*>& global_class_name_map_ref = GetClassNameMap();
	std::unordered_map<std::string, Enum*>& global_enum_name_map_ref = GetEnumNameMap();

	std::vector<Class*> GetRootClasses()
	{
		std::vector<Class*> root_classes;
		for (auto class_name_map_iterator : global_class_name_map_ref)
		{
			if (!class_name_map_iterator.second->GetFirstParentClass())
			{
				root_classes.push_back(class_name_map_iterator.second);
			}
		}
		return root_classes;
	}

	Enum* FindEnum(const std::string& enum_name)
	{
		auto enum_name_map_iterator = global_enum_name_map_ref.find(enum_name);
		if (enum_name_map_iterator != global_enum_name_map_ref.end())
		{
			if (enum_name_map_iterator->second->GetCastTypeFlag() & CTFB_EnumBit)
			{
				return static_cast<Enum*>(enum_name_map_iterator->second);
			}
		}
		return nullptr;
	}

	Class* FindClass(const std::string& class_name)
	{
		auto class_name_map_iterator = global_class_name_map_ref.find(class_name);
		if (class_name_map_iterator != global_class_name_map_ref.end())
		{
			if (class_name_map_iterator->second->GetCastTypeFlag() & CTFB_ClassBit)
			{
				return static_cast<Class*>(class_name_map_iterator->second);
			}
		}
		return nullptr;
	}

	std::unordered_map<std::type_index, Type*>& GetTypeIndexMap()
	{
		static std::unordered_map<std::type_index, Type*> static_type_index_map;
		return static_type_index_map;
	}
	std::unordered_map<std::type_index, Type*>& global_type_index_map = GetTypeIndexMap();

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

	std::vector<std::unique_ptr<ReferenceWrapperType>>& GetStaticReferenceWrapperTypes()
	{
		static std::vector<std::unique_ptr<ReferenceWrapperType>> static_reference_wrapper_type_vector;
		return static_reference_wrapper_type_vector;
	}

	std::unordered_map<std::type_index, ReferenceWrapperType*>& GetUninitializeReferenceWrapperTypeMap()
	{
		static std::unordered_map<std::type_index, ReferenceWrapperType*> static_uninitialize_reference_wrapper_type_map;
		return static_uninitialize_reference_wrapper_type_map;
	}

	std::unordered_map<std::type_index, Type*>& GetReferenceWrapperTypeWrappedTypeMap()
	{
		static std::unordered_map<std::type_index, Type*> static_reference_wrapper_type_wrapped_type_map;
		return static_reference_wrapper_type_wrapped_type_map;
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


