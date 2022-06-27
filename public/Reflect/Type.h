#pragma once
#include "Record.h"

namespace Reflect
{
    enum ECastTypeFlagBits : uint32_t
    {
        CTFB_NoFlag = 0ULL,
        CTFB_VoidBit = 1ULL << 0,
        CTFB_BoolBit = 1ULL << 1,
        CTFB_FloatBit = 1ULL << 2,
        CTFB_DoubleBit = 1ULL << 3,
        CTFB_SInt8Bit = 1ULL << 4,
        CTFB_SInt16Bit = 1ULL << 5,
        CTFB_SInt32Bit = 1ULL << 6,
        CTFB_SInt64Bit = 1ULL << 7,
        CTFB_UInt8Bit = 1ULL << 8,
        CTFB_UInt16Bit = 1ULL << 9,
        CTFB_UInt32Bit = 1ULL << 10,
        CTFB_UInt64Bit = 1ULL << 11,
        CTFB_StringBit = 1ULL << 12,
        CTFB_EnumBit = 1ULL << 13,
        CTFB_ClassBit = 1ULL << 14,
        CTFB_VectorBit = 1ULL << 16,
        CTFB_PtrMapBit = 1ULL << 17,
        CTFB_Int64MapBit = 1ULL << 18,
        CTFB_StringMapBit = 1ULL << 19,
        CTFB_PtrUnorderedMapBit = 1ULL << 20,
        CTFB_Int64UnorderedMapBit = 1ULL << 21,
        CTFB_StringUnorderedMapBit = 1ULL << 22,

        CTFB_SIntBits = CTFB_SInt8Bit | CTFB_SInt16Bit | CTFB_SInt32Bit | CTFB_SInt64Bit,
        CTFB_UIntBits = CTFB_UInt8Bit | CTFB_UInt16Bit | CTFB_UInt32Bit | CTFB_UInt64Bit,
        CTFB_IntBits = CTFB_SIntBits | CTFB_UIntBits,
        CTFB_FloatBits = CTFB_FloatBit | CTFB_DoubleBit,
    };

    template<typename T> struct TStaticBuiltinTypeFlag    { static constexpr ECastTypeFlagBits Value = ECastTypeFlagBits::CTFB_NoFlag; };
    template<> struct TStaticBuiltinTypeFlag<bool>        { static constexpr ECastTypeFlagBits Value = ECastTypeFlagBits::CTFB_BoolBit;   };
    template<> struct TStaticBuiltinTypeFlag<int8_t>      { static constexpr ECastTypeFlagBits Value = ECastTypeFlagBits::CTFB_SInt8Bit;  };
    template<> struct TStaticBuiltinTypeFlag<int16_t>     { static constexpr ECastTypeFlagBits Value = ECastTypeFlagBits::CTFB_SInt16Bit; };
    template<> struct TStaticBuiltinTypeFlag<int32_t>     { static constexpr ECastTypeFlagBits Value = ECastTypeFlagBits::CTFB_SInt32Bit; };
    template<> struct TStaticBuiltinTypeFlag<int64_t>     { static constexpr ECastTypeFlagBits Value = ECastTypeFlagBits::CTFB_SInt64Bit; };
    template<> struct TStaticBuiltinTypeFlag<uint8_t>     { static constexpr ECastTypeFlagBits Value = ECastTypeFlagBits::CTFB_UInt8Bit;  };
    template<> struct TStaticBuiltinTypeFlag<uint16_t>    { static constexpr ECastTypeFlagBits Value = ECastTypeFlagBits::CTFB_UInt16Bit; };
    template<> struct TStaticBuiltinTypeFlag<uint32_t>    { static constexpr ECastTypeFlagBits Value = ECastTypeFlagBits::CTFB_UInt32Bit; };
    template<> struct TStaticBuiltinTypeFlag<uint64_t>    { static constexpr ECastTypeFlagBits Value = ECastTypeFlagBits::CTFB_UInt64Bit; };
    template<> struct TStaticBuiltinTypeFlag<float>       { static constexpr ECastTypeFlagBits Value = ECastTypeFlagBits::CTFB_FloatBit;  };
    template<> struct TStaticBuiltinTypeFlag<double>      { static constexpr ECastTypeFlagBits Value = ECastTypeFlagBits::CTFB_DoubleBit; };
    template<> struct TStaticBuiltinTypeFlag<std::string> { static constexpr ECastTypeFlagBits Value = ECastTypeFlagBits::CTFB_StringBit; };

    class MYGUI_API Type : public Record
	{
	public:
		Type(const std::string& name = "") : Record(name) {}

        uint32_t GetCastTypeFlag() const { return _CastTypeFlag; }
        uint32_t GetMemorySize() const { return _MemorySize; }

	protected:
		uint32_t _CastTypeFlag;
		uint32_t _MemorySize;

		void* (*New)        (            );
		void  (*Delete)     (void*       );
		void  (*Constructor)(void*       );
		void  (*Destructor) (void*       );
		void  (*CopyAssign) (void*, void*);
		void  (*MoveAssign) (void*, void*);
    private:
        template<typename T> friend Type* GetType();
	};

    template<typename CppType>
    class TType : public Type
    {
    public:
        TType(const std::string& name = "") : Type(name)
        {
            _MemorySize = sizeof(CppType);
            New         = &TType<CppType>::Private_New;
            Delete      = &TType<CppType>::Private_Delete;
            Constructor = &TType<CppType>::Private_Constructor;
            Destructor  = &TType<CppType>::Private_Destructor;
            CopyAssign  = &TType<CppType>::Private_CopyAssign;
            MoveAssign  = &TType<CppType>::Private_MoveAssign;
        }
    private:
        static void* Private_New        ()                 { return new CppType(); }
        static void  Private_Delete     (void* A)          { delete static_cast<CppType*>(A); }
        static void  Private_Constructor(void* A)          { new (A) CppType(); }
        static void  Private_Destructor (void* A)          { ((const CppType*)(A))->~CppType(); }
        static void  Private_CopyAssign (void* A, void* B) { *static_cast<CppType*>(A) = *static_cast<CppType*>(B); }
        static void  Private_MoveAssign (void* A, void* B) { *static_cast<CppType*>(A) = std::move(*static_cast<CppType*>(B)); }
    };

    template<typename T>
    class TSimpleStaticType : public TType<T>
    {
    public:
        TSimpleStaticType(const std::string& name)
            : TType<T>(name)
        {}
    };

    template<typename T> Type* GetStaticType() { return nullptr; }
    MYGUI_API template<> Type* GetStaticType<bool>();
    MYGUI_API template<> Type* GetStaticType<float>();
    MYGUI_API template<> Type* GetStaticType<double>();
    MYGUI_API template<> Type* GetStaticType<int8_t>();
    MYGUI_API template<> Type* GetStaticType<int16_t>();
    MYGUI_API template<> Type* GetStaticType<int32_t>();
    MYGUI_API template<> Type* GetStaticType<int64_t>();
    MYGUI_API template<> Type* GetStaticType<uint8_t>();
    MYGUI_API template<> Type* GetStaticType<uint16_t>();
    MYGUI_API template<> Type* GetStaticType<uint32_t>();
    MYGUI_API template<> Type* GetStaticType<uint64_t>();
    MYGUI_API template<> Type* GetStaticType<std::string>();

    MYGUI_API template<> Type* GetStaticType<std::vector<std::any>>();
    MYGUI_API template<> Type* GetStaticType<std::map<void*, std::any>>();
    MYGUI_API template<> Type* GetStaticType<std::map<int64_t, std::any>>();
    MYGUI_API template<> Type* GetStaticType<std::map<std::string, std::any>>();
    MYGUI_API template<> Type* GetStaticType<std::unordered_map<void*, std::any>>();
    MYGUI_API template<> Type* GetStaticType<std::unordered_map<int64_t, std::any>>();
    MYGUI_API template<> Type* GetStaticType<std::unordered_map<std::string, std::any>>();

    class Enum;
    template<typename T> Enum* GetStaticEnum() { return nullptr; }

    MYGUI_API std::unordered_map<std::type_index, Type*>& GetTypeIndexMap();

    MYGUI_API Type* GetType(const std::type_info& type_info);

    class Class;
    template<typename T>
    struct IsReflectClassType
    {
    private:
        template<typename U, Class* (*)()> struct Check {};
        template<class C> static std::true_type Test(Check<C, &C::StaticClass>*);
        template<class C> static std::false_type Test(...);
    public:
        static constexpr bool value = std::is_same<std::true_type, decltype(Test<T>(nullptr))>::value;
    };

    template<typename T>
    constexpr bool ConstexprTrue = true;
    template<typename T>
    constexpr bool ConstexprFalse = false;

    template<typename T>
    Type* GetType()
    {
        static_assert(!std::is_reference_v<T>);
        std::unordered_map<std::type_index, Type*>& type_index_map_ref = GetTypeIndexMap();
        std::type_index type_index = std::type_index(typeid(T));
        auto type_index_map_iterator = type_index_map_ref.find(type_index);
        if (type_index_map_ref.end() != type_index_map_iterator) return type_index_map_iterator->second;
        Type* return_type = nullptr;
        if constexpr (std::is_arithmetic_v<T>)
        {
            return_type = GetStaticType<T>();
            return_type->_CastTypeFlag = TStaticBuiltinTypeFlag<T>::Value;
        }
        else if constexpr (std::is_same_v<T, std::string>)
        {
            return_type = GetStaticType<std::string>();
            return_type->_CastTypeFlag = ECastTypeFlagBits::CTFB_StringBit;
        }
        else if constexpr (std::is_class_v<T> && IsReflectClassType<T>::value)
        {
            return_type = T::StaticClass();
            return_type->_CastTypeFlag = ECastTypeFlagBits::CTFB_ClassBit;
        }
        else if constexpr (std::is_enum_v<T>)
        {
            return_type = GetStaticEnum<T>();
            return_type->_CastTypeFlag = ECastTypeFlagBits::CTFB_EnumBit;
        }
        else if constexpr (std::is_same_v<T, std::vector<std::any>>)
        {
            return_type = GetStaticType<T>();
            return_type->_CastTypeFlag = ECastTypeFlagBits::CTFB_VectorBit;
        }
        else if constexpr (std::is_same_v<T, std::map<void*, std::any>>)
        {
            return_type = GetStaticType<T>();
            return_type->_CastTypeFlag = ECastTypeFlagBits::CTFB_PtrMapBit;
        }
        else if constexpr (std::is_same_v<T, std::map<int64_t, std::any>>)
        {
            return_type = GetStaticType<T>();
            return_type->_CastTypeFlag = ECastTypeFlagBits::CTFB_Int64MapBit;
        }
        else if constexpr (std::is_same_v<T, std::map<std::string, std::any>>)
        {
            return_type = GetStaticType<T>();
            return_type->_CastTypeFlag = ECastTypeFlagBits::CTFB_StringMapBit;
        }
        else if constexpr (std::is_same_v<T, std::unordered_map<void*, std::any>>)
        {
            return_type = GetStaticType<T>();
            return_type->_CastTypeFlag = ECastTypeFlagBits::CTFB_PtrUnorderedMapBit;
        }
        else if constexpr (std::is_same_v<T, std::unordered_map<int64_t, std::any>>)
        {
            return_type = GetStaticType<T>();
            return_type->_CastTypeFlag = ECastTypeFlagBits::CTFB_Int64UnorderedMapBit;
        }
        else if constexpr (std::is_same_v<T, std::unordered_map<std::string, std::any>>)
        {
            return_type = GetStaticType<T>();
            return_type->_CastTypeFlag = ECastTypeFlagBits::CTFB_StringUnorderedMapBit;
        }
        else
        {
            static_assert(ConstexprFalse<T> && "UNSUPPORTED TYPE");
        }
        type_index_map_ref[type_index] = return_type;
        return return_type;
    }

}









