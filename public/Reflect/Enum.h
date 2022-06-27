#pragma once
#include "Type.h"


namespace Reflect
{

	struct EnumValue
	{
		uint64_t _Value{ 0 };
		std::string _CppName;
		std::string _DisplayName;
	};

	MYGUI_API extern EnumValue EmptyEnumValue;

	class MYGUI_API Enum : public Type
	{
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

	protected:
		std::map<uint64_t, EnumValue> _EnumValueMap;

	private:
		template<typename T> friend struct TCustomReflectEnumModifier;
		template<typename T> friend struct TDefaultReflectEnumInitializer;
	};

	template<typename T>
	struct TCustomReflectEnumModifier {
		void operator()(Enum* initialized_enum) {}
	};
	template<typename T>
	struct TDefaultReflectEnumInitializer {
		void operator()(Enum* uninitialized_enum) {}
	};

	template<typename T>
	struct TReflectEnumAutoInitializer {
		TReflectEnumAutoInitializer() {
			Enum* reflect_enum = GetStaticEnum<T>();
			TDefaultReflectEnumInitializer<T>()(reflect_enum);
			TCustomReflectEnumModifier<T>()(reflect_enum);
		}
	};

	// @test begin
	enum ETestEnum
	{
		TE_0,
		TE_1,
		TE_2,
		TE_3,
		TE_4,
		TE_5,
		TE_6,
	};

	MYGUI_API template<> Enum* GetStaticEnum<ETestEnum>();
	// @test end
};
















