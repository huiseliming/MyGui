#pragma once
#include "Record.h"


namespace Reflect
{
    enum ECastTypeFlag : uint32_t
    {
        CTF_NoFlag = 0ULL,
        CTF_Void   = 1ULL << 0,
        CTF_Bool   = 1ULL << 1,
        CTF_SInt8  = 1ULL << 2,
        CTF_SInt16 = 1ULL << 3,
        CTF_SInt32 = 1ULL << 4,
        CTF_SInt64 = 1ULL << 5,
        CTF_UInt8  = 1ULL << 6,
        CTF_UInt16 = 1ULL << 7,
        CTF_UInt32 = 1ULL << 8,
        CTF_UInt64 = 1ULL << 9,
        CTF_Float  = 1ULL << 10,
        CTF_Double = 1ULL << 11,
        CTF_String = 1ULL << 12,
        CTF_Enum   = 1ULL << 13,
        CTF_Class  = 1ULL << 14,
        CTF_Vector = 1ULL << 16,
        CTF_Set = 1ULL << 17,
        CTF_UnorderedSet = 1ULL << 18,
        CTF_Map = 1ULL << 19,
        CTF_UnorderedMap = 1ULL << 20,

        CTF_SIntBits = CTF_SInt8 | CTF_SInt16 | CTF_SInt32 | CTF_SInt64,
        CTF_UIntBits = CTF_UInt8 | CTF_UInt16 | CTF_UInt32 | CTF_UInt64,
        CTF_IntBits = CTF_SIntBits | CTF_UIntBits,
        CTF_FloatBits = CTF_Float | CTF_Double,
    };

    template<typename T> struct TStaticBuiltinTypeFlag    { static constexpr ECastTypeFlag Value = ECastTypeFlag::CTF_NoFlag; };
    template<> struct TStaticBuiltinTypeFlag<bool>        { static constexpr ECastTypeFlag Value = ECastTypeFlag::CTF_Bool;   };
    template<> struct TStaticBuiltinTypeFlag<int8_t>      { static constexpr ECastTypeFlag Value = ECastTypeFlag::CTF_SInt8;  };
    template<> struct TStaticBuiltinTypeFlag<int16_t>     { static constexpr ECastTypeFlag Value = ECastTypeFlag::CTF_SInt16; };
    template<> struct TStaticBuiltinTypeFlag<int32_t>     { static constexpr ECastTypeFlag Value = ECastTypeFlag::CTF_SInt32; };
    template<> struct TStaticBuiltinTypeFlag<int64_t>     { static constexpr ECastTypeFlag Value = ECastTypeFlag::CTF_SInt64; };
    template<> struct TStaticBuiltinTypeFlag<uint8_t>     { static constexpr ECastTypeFlag Value = ECastTypeFlag::CTF_UInt8;  };
    template<> struct TStaticBuiltinTypeFlag<uint16_t>    { static constexpr ECastTypeFlag Value = ECastTypeFlag::CTF_UInt16; };
    template<> struct TStaticBuiltinTypeFlag<uint32_t>    { static constexpr ECastTypeFlag Value = ECastTypeFlag::CTF_UInt32; };
    template<> struct TStaticBuiltinTypeFlag<uint64_t>    { static constexpr ECastTypeFlag Value = ECastTypeFlag::CTF_UInt64; };
    template<> struct TStaticBuiltinTypeFlag<float>       { static constexpr ECastTypeFlag Value = ECastTypeFlag::CTF_Float;  };
    template<> struct TStaticBuiltinTypeFlag<double>      { static constexpr ECastTypeFlag Value = ECastTypeFlag::CTF_Double; };
    template<> struct TStaticBuiltinTypeFlag<std::string> { static constexpr ECastTypeFlag Value = ECastTypeFlag::CTF_String; };

    class Type : public Record
	{
	public:
		Type(const std::string& name = "") : Record(name) {}

	protected:
		uint32_t _TypeCastFlag;
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
    template<> Type* GetStaticType<bool>();
    template<> Type* GetStaticType<int8_t>();
    template<> Type* GetStaticType<int16_t>();
    template<> Type* GetStaticType<int32_t>();
    template<> Type* GetStaticType<int64_t>();
    template<> Type* GetStaticType<uint8_t>();
    template<> Type* GetStaticType<uint16_t>();
    template<> Type* GetStaticType<uint32_t>();
    template<> Type* GetStaticType<uint64_t>();
    template<> Type* GetStaticType<float>();
    template<> Type* GetStaticType<double>();
    template<> Type* GetStaticType<std::string>();

    std::unordered_map<std::type_index, Type*>& GetTypeIndexMap();

    template<typename T>
    Type* GetType()
    {
        static_assert(!std::is_reference_v<T>);
        std::unordered_map<std::type_index, Type*>& type_index_map_ref = GetTypeIndexMap();
        std::type_index type_index = std::type_index(typeid(T));
        auto It = type_index_map_ref.find(type_index);
        if (type_index_map_ref.end() != It) return It->second;
        Type* return_type = nullptr;
        //if constexpr (std::is_class_v<T>)
        //{
        //    return_type = T::StaticClass();
        //    return_type->_TypeCastFlag = ECastTypeFlag::CTF_Class;
        //}
        //else if constexpr (std::is_arithmetic_v<T>)
        //{
        //    return_type = GetBuiltinType<T>();
        //    return_type->_TypeCastFlag = TStaticBuiltinTypeFlag<T>::Value;
        //}
        //else if constexpr (std::is_same_v<T, std::string>)
        //{
        //    return_type = GetStringType();
        //    return_type->_TypeCastFlag = ECastTypeFlag::CTF_String;
        //}
        //else if constexpr (std::is_enum_v<T>)
        //{
        //    return_type = GetEnumType<T>();
        //    return_type->_TypeCastFlag = ECastTypeFlag::CTF_Enum;
        //}
        //else if constexpr (IsStdVector<T>::value)
        //{
        //    return_type = new TStdVectorContainer<IsStdVector<T>::ElementType>();
        //    return_type->_TypeCastFlag = ECastTypeFlag::CTF_Vector;
        //}
        //else if constexpr (IsStdSet<T>::value)
        //{
        //    return_type = new TStdSetContainer<IsStdSet<T>::ElementType>();
        //    return_type->_TypeCastFlag = ECastTypeFlag::CTF_Set;
        //}
        //else if constexpr (IsStdMap<T>::value)
        //{
        //    return_type = new TStdMapContainer<IsStdMap<T>::KeyType, IsStdMap<T>::ValueType>();
        //    return_type->_TypeCastFlag = ECastTypeFlag::CTF_Map;
        //}
        //else
        //{
        //    static_assert(GetReflTypeNotSupported<T> && "UNSUPPORTED TYPE");
        //}
        //RetReflType->Register();
        type_index_map_ref[type_index] = return_type;
        return return_type;
    }
}









