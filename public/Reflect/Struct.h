#pragma once
#include "Type.h"
#include "Field.h"

namespace Reflect
{
	class Class;

	class MYGUI_API Struct : public Type
	{
	public:
		Struct(const std::string& name = "") : Type(name) {}

		const std::vector<std::unique_ptr<Field>>& GetFields() { return _Fields; }

	protected:
		std::vector<std::unique_ptr<Field>> _Fields;

	};

	template<typename T>
	struct TCustomClassModifier {
		void operator()(Class* initialized_class) {}
	};
	template<typename T>
	struct TDefaultClassInitializer {
		void operator()(Class* uninitialized_class) {}
	};

	template<typename T>
	struct TClassAutoInitializer {
		TClassAutoInitializer() {
			Class* reflect_class = T::StaticClass();
			TDefaultClassInitializer<T>()(reflect_class);
			TCustomClassModifier<T>()(reflect_class);
		}
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
