#pragma once
#include "Record.h"
#include <spdlog/spdlog.h>

#define IMPL_DEFAULT_TYPE_BASE_FUNCTION(CppType)                                                                                                             \
_MemorySize  = sizeof(CppType);                                                                                                                              \
_New         = []() -> void* { return new CppType(); };                                                                                                      \
_Delete      = [](void* A) { delete static_cast<CppType*>(A); };                                                                                             \
_Constructor = [](void* A) { new (A) CppType(); };                                                                                                           \
_Destructor  = [](void* A) { ((const CppType*)(A))->~CppType(); };                                                                                           \
if constexpr (std::is_copy_assignable_v<CppType>) _CopyAssign  = [](void* A, void* B) { *static_cast<CppType*>(A) = *static_cast<CppType*>(B); };            \
if constexpr (std::is_move_assignable_v<CppType>) _MoveAssign  = [](void* A, void* B) { *static_cast<CppType*>(A) = std::move(*static_cast<CppType*>(B)); }; \

#define IMPL_DEFAULT_TYPE_ANY_ACCESSOR_FUNCTION(CppType)                                    \
if constexpr (std::is_copy_assignable_v<CppType>)                                           \
{                                                                                           \
    _GetAny = [](std::any& any_ref) -> void* { return std::any_cast<CppType>(&any_ref); };  \
    _SetAny = [](std::any& any_ref, void* data_ptr) { any_ref = *(CppType*)data_ptr; };     \
}                                                                                           \

namespace Core
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

        CTFB_ReferenceWrapperBit = 1ULL << 30,
        CTFB_PointerBit = 1ULL << 31,
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

    class MYGUI_API CLASS() Type : public Record
	{
        GENERATED_OBJECT_BODY()
	public:
		Type(const std::string& name = "") : Record(name) {}

        uint32_t GetCastTypeFlag() const { return _CastTypeFlag; }
        uint32_t GetMemorySize() const { return _MemorySize; }

        void* New() { return _New(); }
        void  Delete(void* A) { _Delete(A); }
        void  Constructor(void* A) { _Constructor(A); }
        void  Destructor(void* A) { _Destructor(A); }
        void  CopyAssign(void* A, void* B) { _CopyAssign(A, B); }
        void  MoveAssign(void* A, void* B) { _MoveAssign(A, B); }
        void* GetAny(std::any& any_ref) { return _GetAny(any_ref); }
        void  SetAny(std::any& any_ref, void* data_ptr) { _SetAny(any_ref, data_ptr); }
        template<typename T>
        T& GetAnyAs(std::any& any_ref) { return *static_cast<T*>(GetAny(any_ref)); }
        template<typename T>
        void SetAnyBy(std::any& any_ref, T& data_ref) { return SetAny(any_ref, &data_ref); }

        virtual Type* GetPointToType() const { return nullptr; }
        virtual Type* GetWrappedType() const { return nullptr; }

	protected:
		uint32_t _CastTypeFlag;
		uint32_t _MemorySize;

        void* (*_New)() { nullptr };
        void (*_Delete) (void*) { nullptr };
        void (*_Constructor) (void*) { nullptr };
        void (*_Destructor) (void*) { nullptr };
        void (*_CopyAssign) (void*, void*) { nullptr };
        void (*_MoveAssign) (void*, void*) { nullptr };
        void* (*_GetAny) (std::any& any_ref) { nullptr };
        void (*_SetAny) (std::any& any_ref, void* data_ptr) { nullptr };
    private:
        template<typename T> friend Type* GetType();
	};

    template<typename CppType>
    class TType : public Type
    {
    public:
        TType(const std::string& name = "") : Type(name)
        {
            IMPL_DEFAULT_TYPE_BASE_FUNCTION(CppType);
            IMPL_DEFAULT_TYPE_ANY_ACCESSOR_FUNCTION(CppType);
        }
    };

    template<typename T>
    class TSimpleStaticType : public TType<T>
    {
    public:
        TSimpleStaticType(const std::string& name = "")
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

    MYGUI_API std::unordered_map<std::string, Class*>& GetClassNameMap();
    MYGUI_API std::unordered_map<std::string, Enum*>& GetEnumNameMap();
    MYGUI_API extern std::unordered_map<std::string, Class*>& global_class_name_map_ref;
    MYGUI_API extern std::unordered_map<std::string, Enum*>& global_enum_name_map_ref;

    MYGUI_API std::vector<Class*> GetRootClasses();

    MYGUI_API Enum* FindEnum(const std::string& enum_name);
    MYGUI_API Class* FindClass(const std::string& class_name);

    MYGUI_API std::unordered_map<std::type_index, Type*>& GetTypeIndexMap();
    MYGUI_API extern std::unordered_map<std::type_index, Type*>& global_type_index_map;

    MYGUI_API Type* GetType(const std::type_info& type_info);

    // 
    class PointerType : public Type
    {
        using CppType = void*;
    public:
        PointerType(const std::string& name = "")
            : Type(name)
        {
            IMPL_DEFAULT_TYPE_BASE_FUNCTION(CppType)
            IMPL_DEFAULT_TYPE_ANY_ACCESSOR_FUNCTION(CppType)
        }

        virtual Type* GetPointToType() const override { return _PointToType; }
    private:
        Type* _PointToType{ nullptr };
    private:
        template<typename T> friend Type* GetType();
    };

    MYGUI_API std::vector<std::unique_ptr<PointerType>>& GetStaticPointerTypes();
    MYGUI_API std::unordered_map<std::type_index, PointerType*>& GetUninitializePointerTypeMap();
    MYGUI_API std::unordered_map<std::type_index, Type*>& GetPointerTypePointToTypeIndexMap();

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

    // 

    template<typename T>
    struct IsReferenceWrapperType : std::false_type
    {
        using WrappedType = void;
    };
    template<typename T>
    struct IsReferenceWrapperType<std::reference_wrapper<T>> : std::true_type
    {
        using WrappedType = T;
    };

    class ReferenceWrapperType : public Type
    {
    public:
        ReferenceWrapperType(const std::string& name = "")
            : Type(name)
        {}

        virtual Type* GetWrappedType() const override { return _WrappedType; }

    private:
        Type* _WrappedType{ nullptr };

    private:
        template<typename T> friend Type* GetType();
    };

    template<typename CppType>
    class TReferenceWrapperType : public ReferenceWrapperType
    {
    public:
        TReferenceWrapperType(const std::string& name = "")
            : ReferenceWrapperType(name)
        {
            using WrappedType = IsReferenceWrapperType<CppType>::WrappedType;
            _MemorySize = sizeof(CppType);
            _New = []() -> void* { return new CppType(*(std::add_pointer_t<WrappedType>)nullptr); };
            _Delete = [](void* A) { delete static_cast<CppType*>(A); };
            _Constructor = [](void* A) { new (A) CppType(*(std::add_pointer_t<WrappedType>)nullptr); };
            _Destructor = [](void* A) { ((const CppType*)(A))->~CppType(); };
            _CopyAssign = [](void* A, void* B) { *static_cast<CppType*>(A) = *static_cast<CppType*>(B); };
            _MoveAssign = [](void* A, void* B) { *static_cast<CppType*>(A) = std::move(*static_cast<CppType*>(B)); };
            _GetAny = [](std::any& any_ref) -> void* { return std::any_cast<CppType*>(&any_ref); };
            _SetAny = [](std::any& any_ref, void* data_ptr) { any_ref = *(CppType*)data_ptr; };

        }
    };

    template<typename T>
    using ConvertToRefWrapper = 
        std::conditional_t<
            std::is_reference_v<T>, 
            std::reference_wrapper<std::remove_cvref_t<T>>,
            std::remove_cv_t<T>
        >;

    MYGUI_API std::vector<std::unique_ptr<ReferenceWrapperType>>& GetStaticReferenceWrapperTypes();
    MYGUI_API std::unordered_map<std::type_index, ReferenceWrapperType*>& GetUninitializeReferenceWrapperTypeMap();
    MYGUI_API std::unordered_map<std::type_index, Type*>& GetReferenceWrapperTypeWrappedTypeMap();

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

        auto& uninitialize_pointer_type_map_ref = GetUninitializePointerTypeMap();
        auto& pointer_type_point_to_type_index_map_ref = GetPointerTypePointToTypeIndexMap();

        auto& uninitialize_reference_wrapper_type_map_ref = GetUninitializeReferenceWrapperTypeMap();
        auto& reference_wrapper_type_wrapped_type_map_ref = GetReferenceWrapperTypeWrappedTypeMap();

        Type* return_type = nullptr;
        if constexpr (IsReferenceWrapperType<T>::value)
        {
            using WrappedType = IsReferenceWrapperType<T>::WrappedType;
            auto& static_reference_wrapper_types_ref = GetStaticReferenceWrapperTypes();
            static_reference_wrapper_types_ref.push_back(std::make_unique<TReferenceWrapperType<T>>());
            auto static_reference_wrapper_type_ptr = static_reference_wrapper_types_ref.back().get();
            auto reference_wrapper_type_wrapped_type_map_iterator = reference_wrapper_type_wrapped_type_map_ref.find(type_index);
            if (reference_wrapper_type_wrapped_type_map_iterator != reference_wrapper_type_wrapped_type_map_ref.end())
            {
                static_reference_wrapper_type_ptr->_WrappedType = reference_wrapper_type_wrapped_type_map_iterator->second;
            }
            else
            {
                uninitialize_reference_wrapper_type_map_ref.insert(std::make_pair(type_index, static_reference_wrapper_type_ptr));
            }
            return_type = static_reference_wrapper_type_ptr;
            return_type->_CastTypeFlag = ECastTypeFlagBits::CTFB_ReferenceWrapperBit;
        }
        else if constexpr (std::is_pointer_v<T>)
        {
            auto& static_pointer_types_ref = GetStaticPointerTypes();
            static_pointer_types_ref.push_back(std::make_unique<PointerType>());
            auto static_pointer_type_ptr = static_pointer_types_ref.back().get();
            auto pointer_type_point_to_type_index_map_iterator = pointer_type_point_to_type_index_map_ref.find(type_index);
            if (pointer_type_point_to_type_index_map_iterator != pointer_type_point_to_type_index_map_ref.end())
            {
                static_pointer_type_ptr->_PointToType = pointer_type_point_to_type_index_map_iterator->second;
            }
            else
            {
                uninitialize_pointer_type_map_ref.insert(std::make_pair(type_index, static_pointer_type_ptr));
            }
            return_type = static_pointer_type_ptr;
            return_type->_CastTypeFlag = ECastTypeFlagBits::CTFB_PointerBit;
        }
        else if constexpr (std::is_arithmetic_v<T>)
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
            GetClassNameMap().insert_or_assign(return_type->GetName(), static_cast<Class*>(return_type));
        }
        else if constexpr (std::is_enum_v<T>)
        {
            return_type = GetStaticEnum<T>();
            return_type->_CastTypeFlag = ECastTypeFlagBits::CTFB_EnumBit;
            GetEnumNameMap().insert_or_assign(return_type->GetName(), static_cast<Enum*>(return_type));
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
        // pointer type 
        {
            using AddPointerType = std::add_pointer_t<T>;
            std::type_index add_pointer_type_index = typeid(AddPointerType);
            pointer_type_point_to_type_index_map_ref.insert(std::make_pair(add_pointer_type_index, return_type));
            auto uninitialize_pointer_type_map_iterator = uninitialize_pointer_type_map_ref.find(add_pointer_type_index);
            if (uninitialize_pointer_type_map_iterator != uninitialize_pointer_type_map_ref.end())
            {
                uninitialize_pointer_type_map_iterator->second->_PointToType = return_type;
                uninitialize_pointer_type_map_ref.erase(uninitialize_pointer_type_map_iterator);
            }
        }
        // reference wrapper type
        {
            using ReferenceWrapperType = std::reference_wrapper<T>;
            std::type_index reference_wrapper_type_index = typeid(ReferenceWrapperType);
            reference_wrapper_type_wrapped_type_map_ref.insert(std::make_pair(reference_wrapper_type_index, return_type));
            auto uninitialize_reference_wrapper_type_map_iterator = uninitialize_reference_wrapper_type_map_ref.find(reference_wrapper_type_index);
            if (uninitialize_reference_wrapper_type_map_iterator != uninitialize_reference_wrapper_type_map_ref.end())
            {
                uninitialize_reference_wrapper_type_map_iterator->second->_WrappedType = return_type;
                uninitialize_reference_wrapper_type_map_ref.erase(uninitialize_reference_wrapper_type_map_iterator);
            }
        }
        type_index_map_ref[type_index] = return_type;
        return return_type;
    }

    template<typename T>
    struct TCustomTypeModifier {
        void operator()(Type* initialized_type) {}
    };
    template<typename T>
    struct TDefaultTypeInitializer {
        void operator()(Type* uninitialized_type) {}
    };

    template<typename T>
    struct TTypeAutoInitializer {
        TTypeAutoInitializer() {
            Type* reflect_type = GetType<T>();
            TDefaultTypeInitializer<T>()(reflect_type);
            TCustomTypeModifier<T>()(reflect_type);
        }
    };

    MYGUI_API bool VerifyStaticTypeInitializationResult();

}









