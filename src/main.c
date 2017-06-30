#include <clanguml/clanguml.h>
#include <unistd.h>
#include <stdio.h>

struct uml_context
{
    FILE* fd;
};

static void packageBegin(void* user_data, CXCursor cursor)
{
    struct uml_context* ctx = (struct uml_context*)user_data;
    const char* packageName = clang_getCString(clang_getCursorSpelling(cursor));
    fprintf(ctx->fd,"package %s {\n",packageName);
}
static void packageEnd(void* user_data, CXCursor cursor)
{
    struct uml_context* ctx = (struct uml_context*)user_data;
    fprintf(ctx->fd,"}\n");
}
static void classBegin(void* user_data, CXCursor cursor, ClangUML_classType classType,  const char* stereotype)
{
    struct uml_context* ctx = (struct uml_context*)user_data;
    const char* className = clang_getCString(clang_getCursorSpelling(cursor));
    switch (classType)
    {
    case ClangUML_classType_enum:
        fprintf(ctx->fd,"enum %s {\n",className);
        break;
    case ClangUML_classType_class:
        fprintf(ctx->fd,"class %s {\n",className);
        break;
    }
}
static void classEnd(void* user_data, CXCursor cursor)
{
    struct uml_context* ctx = (struct uml_context*)user_data;
    fprintf(ctx->fd,"}\n");
}
static void methodEnter(void* user_data, CXCursor cursor, const ClangUML_visibility vis, const ClangUML_classifier cla)
{
    struct uml_context* ctx = (struct uml_context*)user_data;
    const char* methodName = clang_getCString(clang_getCursorDisplayName(cursor));
    fprintf(ctx->fd,"    %s\n", methodName);
}
static void methodLeave(void* user_data, CXCursor cursor)
{
    struct uml_context* ctx = (struct uml_context*)user_data;

}
static void field(void* user_data, CXCursor cursor, const char* fieldType, const ClangUML_visibility vis, const ClangUML_classifier cla )
{
    struct uml_context* ctx = (struct uml_context*)user_data;
    CXString displayName = clang_getCursorDisplayName(cursor);
    char visibilityType = '+';
    switch(vis)
    {
    case ClangUML_visibility_public:
        visibilityType = '+';
        break;
    case ClangUML_visibility_protected:
        visibilityType = '#';
        break;
    case ClangUML_visibility_private:
        visibilityType = '-';
        break;
    }
    fprintf(ctx->fd,"    %c%s : %s\n", visibilityType, clang_getCString(displayName), fieldType);
    clang_disposeString(displayName);
}
static void relationship(void* user_data, CXCursor cursor, const char* other, ClangUML_classRelation rel)
{
    struct uml_context* ctx = (struct uml_context*)user_data;
    CXString displayName = clang_getCursorDisplayName(cursor);
    switch(rel)
    {
    case ClangUML_classRelation_extension:
        fprintf(ctx->fd,"%s --|> %s\n", clang_getCString(displayName), other);
        break;
    case ClangUML_classRelation_composition:
        fprintf(ctx->fd,"%s *-- %s\n", clang_getCString(displayName), other);
        break;
    case ClangUML_classRelation_aggregation:
        fprintf(ctx->fd,"%s o-- %s\n", clang_getCString(displayName), other);
        break;
    }
    clang_disposeString(displayName);
}

int main(int argc, char* argv[])
{
    int i;
    int c;
    while ((c = getopt(argc, argv, "h")) != -1)
    {
        switch(c)
        {
            case 'h':
            break;
            case '?':
            break;
            default:
                return -1;
        }
    }
    if (optind == argc)
    {
        fprintf(stderr,"Please pass a filename to parse\n");
        return -1;
    }
    struct uml_context ctx;
    ctx.fd = stdout;
    ClangUML_t uml = ClangUML_new(packageBegin, packageEnd, classBegin,classEnd,methodEnter, methodLeave, field, relationship);
    CXIndex index = clang_createIndex(0, 0);
    fprintf(ctx.fd,"@startuml\n");
    for (i = optind; i < argc; ++i)
    {
        CXTranslationUnit tu = clang_parseTranslationUnit(index, argv[i], NULL, 0, NULL, 0, CXTranslationUnit_None);
	if (!tu)
            fprintf(stderr,"Unable to parse %s\n",argv[i]);
        else
        {
            ClangUML_visitTranslationUnit(uml, tu, &ctx);
            clang_disposeTranslationUnit(tu);
        }
    } 
    fprintf(ctx.fd,"@enduml");
    clang_disposeIndex(index);
    ClangUML_dispose(&uml);
}
