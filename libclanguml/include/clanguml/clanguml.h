#ifndef __LIBCLANGUML_H__
#define __LIBCLANGUML_H__

#include <clang-c/Index.h>

#ifndef CLANGUML_API
#define CLANGUML_API
#endif

typedef enum _ClangUML_classType
{
    ClangUML_classType_enum,
    ClangUML_classType_interface,
    ClangUML_classType_abstract,
    ClangUML_classType_class,
}ClangUML_classType;

typedef enum _ClangUML_classRelation
{
    ClangUML_classRelation_extension,
    ClangUML_classRelation_composition,
    ClangUML_classRelation_aggregation
}ClangUML_classRelation;

typedef enum _ClangUML_visibility
{
    ClangUML_visibility_public,
    ClangUML_visibility_protected,
    ClangUML_visibility_package,
    ClangUML_visibility_private
}ClangUML_visibility;

typedef enum _ClangUML_classifier
{
    ClangUML_classifier_normal,
    ClangUML_classifier_static,
    ClangUML_classifier_abstract,
}ClangUML_classifier;

typedef void* ClangUML_t;

typedef void (*ClangUML_classDiag_packageBeginFunc)(void* user_data, CXCursor cursor );
typedef void (*ClangUML_classDiag_packageEndFunc)(  void* user_data, CXCursor cursor );
typedef void (*ClangUML_classDiag_classBeginFunc)(  void* user_data, CXCursor cursor, ClangUML_classType, const char* stereotype );
typedef void (*ClangUML_classDiag_classEndFunc)(    void* user_data, CXCursor cursor );
typedef void (*ClangUML_classDiag_methodEnterFunc)( void* user_data, CXCursor cursor, const ClangUML_visibility vis, const ClangUML_classifier cla );
typedef void (*ClangUML_classDiag_methodLeaveFunc)( void* user_data, CXCursor cursor );
typedef void (*ClangUML_classDiag_fieldFunc)(       void* user_data, CXCursor cursor, const char* fieldType, const ClangUML_visibility vis, const ClangUML_classifier cla );
typedef void (*ClangUML_classDiag_relationshipFunc)(void* user_data, CXCursor cursor, const char* other, ClangUML_classRelation rel );


CLANGUML_API ClangUML_t ClangUML_new(
        ClangUML_classDiag_packageBeginFunc   packageBegin,
        ClangUML_classDiag_packageEndFunc     packageEnd,
        ClangUML_classDiag_classBeginFunc     classBegin,
        ClangUML_classDiag_classEndFunc       classEnd,
        ClangUML_classDiag_methodEnterFunc    methodEnter,
        ClangUML_classDiag_methodLeaveFunc    methodLeave,
        ClangUML_classDiag_fieldFunc          field,
        ClangUML_classDiag_relationshipFunc   relationship
    );
CLANGUML_API int ClangUML_dispose(ClangUML_t* cu);
CLANGUML_API int ClangUML_visitTranslationUnit(ClangUML_t cu, CXTranslationUnit tu, void* user_data);

#endif // __LIBCLANGUML_H__
