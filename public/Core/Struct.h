#pragma once
#include "Type.h"
#include "Field.h"

namespace Core
{
	class Class;

	class MYCPP_API CLASS() Struct : public Type
	{
		GENERATED_OBJECT_BODY()
	public:
		Struct(const std::string& name = "") : Type(name) {}

		const std::vector<std::unique_ptr<Field>>& GetFields() { return _Fields; }

	protected:
		std::vector<std::unique_ptr<Field>> _Fields;

	};

	template<typename T>
	struct ExistReflectInitializer
	{
	private:
		template<typename U, Struct* (*)()> struct Check {};
		template<class C> static std::true_type Test(Check<C, &C::ReflectRegister>*);
		template<class C> static std::false_type Test(...);
	public:
		static constexpr bool value = std::is_same<std::true_type, decltype(Test<T>(nullptr))>::value;
	};

}
