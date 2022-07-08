#pragma once
#include "Type.h"

namespace Core
{

	struct EnumValue
	{
		uint64_t _Value{ 0 };
		std::string _CppName;
		std::string _DisplayName;
	};

	MYGUI_API extern EnumValue EmptyEnumValue;

	class MYGUI_API CLASS() Enum : public Type
	{
		GENERATED_OBJECT_BODY()
	public:
		Enum(const std::string& name = "")
			: Type(name)
		{}

		const EnumValue& GetEnumValue(uint64_t Value) 
		{
			auto enum_value_iterator = _EnumValueMap.find(Value);
			if (enum_value_iterator != _EnumValueMap.end())
			{
				return enum_value_iterator->second;
			}
			return EmptyEnumValue;
		}

		const std::map<uint64_t, EnumValue>& GetEnumValueMap() { return _EnumValueMap; }

		virtual void SetEnumValue(void* type_instance_ptr, uint64_t value) const {}
		virtual void SetEnumValue(void* type_instance_ptr, int64_t value) const {}
		virtual int64_t GetSIntEnumValue(void* type_instance_ptr) const { return 0; }
		virtual uint64_t GetUIntEnumValue(void* type_instance_ptr) const { return 0; }

	protected:
		std::map<uint64_t, EnumValue> _EnumValueMap;

	private:
		template<typename T> friend struct TCustomTypeModifier;
		template<typename T> friend struct TDefaultTypeInitializer;
	};

	template<typename CppType>
	class TEnum : public Enum
	{
	public:
		TEnum(const std::string& name = "")
			: Enum(name)
		{
			IMPL_DEFAULT_TYPE_BASE_FUNCTION(CppType)
			IMPL_DEFAULT_TYPE_ANY_ACCESSOR_FUNCTION(CppType)
		}
		
		virtual void SetEnumValue(void* type_instance_ptr, uint64_t value) const override
		{
			*static_cast<CppType*>(type_instance_ptr) = static_cast<CppType>(value);
		}
		virtual void SetEnumValue(void* type_instance_ptr, int64_t value) const override 
		{
			*static_cast<CppType*>(type_instance_ptr) = static_cast<CppType>(value);
		}
		virtual int64_t GetSIntEnumValue(void* type_instance_ptr) const override 
		{
			return static_cast<int64_t>(*static_cast<CppType*>(type_instance_ptr));
		}
		virtual uint64_t GetUIntEnumValue(void* type_instance_ptr) const override 
		{
			return static_cast<uint64_t>(*static_cast<CppType*>(type_instance_ptr));
		}

	};

	// @test end
};

















