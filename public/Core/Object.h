#pragma once
#include <string>

#ifdef __RUN_CODE_GENERATOR__
#define METADATA(...)  [[clang::annotate("meta" __VA_OPT__(", ") #__VA_ARGS__)]]// __attribute__((annotate("meta" __VA_OPT__(", ") #__VA_ARGS__)))
#define CLASS(...)     METADATA(__VA_ARGS__)
#define STRUCT(...)    METADATA(__VA_ARGS__)
#define ENUM(...)      METADATA(__VA_ARGS__)
#define FIELD(...)	   METADATA(__VA_ARGS__)
#define FUNCTION(...)  METADATA(__VA_ARGS__)
#else
#define CLASS(...)
#define ENUM(...)
#define FIELD(...)
#define FUNCTION(...)
#define METADATA(...)
#endif

#define GENERATED_CLASS_BODY()                              \
private:													\
template<typename T> friend struct TCustomTypeModifier;	\
template<typename T> friend struct TDefaultTypeInitializer;\
public:														\
	static Class* StaticClass();							\
public:														\

#define GENERATED_OBJECT_BODY()                              \
GENERATED_CLASS_BODY()                                       \
virtual Class* GetClass() override { return StaticClass(); } \


namespace Core
{
	class Class;
	class Enum;

	template<typename T> Enum* GetStaticEnum() { return nullptr; }

	class CLASS() Object 
	{
		GENERATED_CLASS_BODY()
	public:
		virtual Class* GetClass() { return StaticClass(); }
	};

	// @test begin
	enum ENUM() ETestEnum
	{
		TE_0 METADATA(DisplayName = test_0),
		TE_1 METADATA(DisplayName = "test1"),
		TE_2 METADATA(DisplayName = "test2"),
		TE_3 METADATA(DisplayName = 30),
		TE_4 METADATA(DisplayName = 4.0),
		TE_5 METADATA(DisplayName = TestFive),
		TE_6 METADATA(DisplayName = "测试6"),
	};

	template<> Enum* GetStaticEnum<ETestEnum>();

	//MYGUI_API
	//template<>
	//struct TCustomTypeModifier<ETestEnum>
	//{
	//	void operator()(Type* initialized_type);
	//};

	class CLASS() TestObject : public Object
	{
		GENERATED_OBJECT_BODY()
	public:
		TestObject() {}

		FIELD()
		uint64_t _UInt64;
		FIELD()
		int64_t _SInt64;
		FIELD()
		double _Double;
		FIELD()
		ETestEnum _TestEnum;
		FIELD()
		std::string _String;
	};

}

