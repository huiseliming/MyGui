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
	struct ExistReflectInitializer
	{
	private:
		template<typename U, Struct* (*)()> struct Check {};
		template<class C> static std::true_type Test(Check<C, &C::ReflectRegister>*);
		template<class C> static std::false_type Test(...);
	public:
		static constexpr bool value = std::is_same<std::true_type, decltype(Test<T>(nullptr))>::value;
	};

	// @test begin
	class TestStruct
	{
	private:
		template<typename T> friend struct TCustomReflectModifier;
		template<typename T> friend struct TDefaultReflectInitializer;
	public:
		static Class* StaticClass();
	public:

		std::vector<std::any> _Vector;
		int32_t _SInt32;
		bool _Boolean;
	}; 
	// @test end
}
