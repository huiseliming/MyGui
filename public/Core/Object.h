#pragma once

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

	class CLASS() Object 
	{
		GENERATED_CLASS_BODY()
	public:
		virtual Class* GetClass() { return StaticClass(); }
	};
}

