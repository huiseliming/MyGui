#include "Reflect/Enum.h"

namespace Reflect
{
	EnumValue EmptyEnumValue;

	// @test begin
	template<>
	struct TCustomReflectEnumModifier<ETestEnum> {
		void operator()(Enum* initialized_enum) {
			initialized_enum->_EnumValueMap.insert(std::make_pair(ETestEnum::TE_2, EnumValue{ ETestEnum::TE_2, "ETestEnum::TE_2", "ETestEnum::TE_2" }));
		}
	};

	template<>
	struct TDefaultReflectEnumInitializer<ETestEnum> {
		void operator()(Enum* uninitialized_enum) {
			uninitialized_enum->_EnumValueMap.insert(std::make_pair(ETestEnum::TE_0, EnumValue{ ETestEnum::TE_0, "ETestEnum::TE_0", "ETestEnum::TE_0"}));
			uninitialized_enum->_EnumValueMap.insert(std::make_pair(ETestEnum::TE_1, EnumValue{ ETestEnum::TE_1, "ETestEnum::TE_1", "ETestEnum::TE_1" }));
		}
	};

	static struct TReflectEnumAutoInitializer<ETestEnum> STestEnumAutoInitializer;

	template<>
	Enum* GetStaticEnum<ETestEnum>()
	{
		static Enum static_enum;
		return &static_enum;
	}
	// @test end
}

