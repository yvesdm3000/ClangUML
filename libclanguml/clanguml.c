#include <clanguml/clanguml.h>
#include <malloc.h>
#include <clang-c/Documentation.h>
#include <string.h>

#ifndef DEBUG_AST
#define DEBUG_AST 0
#endif

typedef struct _ClangUMLPriv
{
    ClangUML_classDiag_packageBeginFunc packageBegin;
    ClangUML_classDiag_packageEndFunc   packageEnd;
    ClangUML_classDiag_classBeginFunc   classBegin;
    ClangUML_classDiag_classEndFunc     classEnd;
    ClangUML_classDiag_methodEnterFunc  methodEnter;
    ClangUML_classDiag_methodLeaveFunc  methodLeave;
    ClangUML_classDiag_fieldFunc        field;
    ClangUML_classDiag_relationshipFunc relationship;
}ClangUMLPriv;


CLANGUML_API ClangUML_t ClangUML_new(
    ClangUML_classDiag_packageBeginFunc packageBegin,
    ClangUML_classDiag_packageEndFunc   packageEnd,
    ClangUML_classDiag_classBeginFunc   classBegin,
    ClangUML_classDiag_classEndFunc     classEnd,
    ClangUML_classDiag_methodEnterFunc  methodEnter,
    ClangUML_classDiag_methodLeaveFunc  methodLeave,
    ClangUML_classDiag_fieldFunc        field,
    ClangUML_classDiag_relationshipFunc relationship
  )
{
    ClangUMLPriv* priv = (ClangUMLPriv*)malloc(sizeof(ClangUMLPriv));
    priv->packageBegin = packageBegin;
    priv->packageEnd = packageEnd;
    priv->classBegin = classBegin;
    priv->classEnd = classEnd;
    priv->methodEnter = methodEnter;
    priv->methodLeave = methodLeave;
    priv->field = field;
    priv->relationship = relationship;
    return priv;
}

CLANGUML_API int ClangUML_dispose(ClangUML_t* cu)
{
    if (cu)
        free(*cu);
    *cu = NULL;
    return 0;
}

struct ASTCursorRelationship{
    char* other;
    ClangUML_classRelation rel;
    struct ASTCursorRelationship* next; // Linked list to next
};

struct ASTVisitorCtx{
    ClangUMLPriv* clangUML;
    void* user_data;
    ClangUML_visibility currentVisibility;
    struct ASTCursorRelationship* relationships;
};


static void ASTCursorRelationship_add(struct ASTVisitorCtx* ctx, const char* name, ClangUML_classRelation relation)
{
    struct ASTCursorRelationship* rel = (struct ASTCursorRelationship*)malloc(sizeof(struct ASTCursorRelationship));
    rel->other = strdup(name);
    rel->rel = relation;
    rel->next = NULL;
    if (ctx->relationships == NULL)
    {
        ctx->relationships = rel;
    }
    else
    {
        struct ASTCursorRelationship* last = ctx->relationships;
        while( last->next != NULL )
        {
            if ((last->rel == relation)&&(strcmp(last->other, name) == 0))
            {
                free(rel->other);
                free(rel);
                return;
            }
            last = last->next;
        }   
        if ((last->rel == relation)&&(strcmp(last->other, name) == 0))
        {
            free(rel->other);
            free(rel);
            return;
        }
        last->next = rel;
    }
}
static enum CXChildVisitResult ASTVisitor(CXCursor cursor, CXCursor parent, CXClientData client_data)
{
    if (clang_Location_isFromMainFile(clang_getCursorLocation(cursor)) == 0) 
    {
        return CXChildVisit_Continue;
    }
    struct ASTVisitorCtx* ctx = (struct ASTVisitorCtx*)client_data;
    enum CXChildVisitResult ret = CXChildVisit_Break; // should never happen
    CXString cursorName = clang_getCursorSpelling(cursor);
    switch (cursor.kind)
    {
    case CXCursor_ClassDecl:
    case CXCursor_ClassTemplate:
        ctx->currentVisibility = ClangUML_visibility_private;
        // fallthru intended
    case CXCursor_StructDecl:
#if DEBUG_AST      
        fprintf(stderr,"StructDecl/ClassDecl/ClassTemplate '%s'\n", clang_getCString(cursorName));
#endif
        if (clang_isCursorDefinition(cursor))
        {
            if (*clang_getCString(cursorName))
            {
                if(ctx->clangUML->classBegin)
                {
                    ctx->clangUML->classBegin(ctx->user_data, cursor, ClangUML_classType_class, NULL);
                }
                clang_visitChildren(cursor,ASTVisitor, client_data);
                if(ctx->clangUML->classEnd)
                {
                    ctx->clangUML->classEnd(ctx->user_data, cursor);
                }
                ret = CXChildVisit_Continue;
            }
            else
            {
                ret = CXChildVisit_Recurse;
            }
        }
        else
        {
            ret = CXChildVisit_Continue;
        }
        ctx->currentVisibility = ClangUML_visibility_public;

        while (ctx->relationships)
        {
            struct ASTCursorRelationship* rel = ctx->relationships;
            if (ctx->clangUML->relationship)
            {
                ctx->clangUML->relationship(ctx->user_data, cursor, rel->other, rel->rel);
            }
            ctx->relationships = ctx->relationships->next;
            free(rel->other);
            free(rel);
        }

        break;
    case CXCursor_UnionDecl:
#if DEBUG_AST      
        fprintf(stderr,"Union %s\n", clang_getCString(cursorName));
#endif
        ret = CXChildVisit_Recurse;
        break;
    case CXCursor_EnumDecl:
#if DEBUG_AST      
        fprintf(stderr,"Enum %s\n", clang_getCString(cursorName));
#endif
        if(ctx->clangUML->classBegin)
        {
            ctx->clangUML->classBegin(ctx->user_data, cursor, ClangUML_classType_enum , NULL);
        }
        clang_visitChildren(cursor,ASTVisitor, client_data);
        if(ctx->clangUML->classEnd)
        {
            ctx->clangUML->classEnd(ctx->user_data, cursor);
        }
    

        ret = CXChildVisit_Continue;
        break;
    case CXCursor_EnumConstantDecl:
#if DEBUG_AST      
        fprintf(stderr,"EnumConstantDecl %s\n", clang_getCString(cursorName));
#endif
        if(ctx->clangUML->field)
        {
            ctx->clangUML->field(ctx->user_data, cursor, NULL, ctx->currentVisibility, ClangUML_classifier_normal );
        }
        ret = CXChildVisit_Continue;
        break;
    case CXCursor_Namespace:
#if DEBUG_AST      
        fprintf(stderr,"Namespace %s\n", clang_getCString(cursorName));
#endif
        if(ctx->clangUML->packageBegin)
        {
            ctx->clangUML->packageBegin(ctx->user_data, cursor);
        }
        clang_visitChildren(cursor,ASTVisitor, client_data);
        if(ctx->clangUML->packageEnd)
        {
            ctx->clangUML->packageEnd(ctx->user_data, cursor);
        }
        ret = CXChildVisit_Continue;
        break;

    case CXCursor_DeclRefExpr:
#if DEBUG_AST      
        fprintf(stderr,"DeclRefExpr %s\n", clang_getCString(cursorName));
#endif
        ret = CXChildVisit_Recurse;
        break;
    case CXCursor_CXXAccessSpecifier:
        {
            switch( clang_getCXXAccessSpecifier(cursor) )
            {
            case CX_CXXPublic:
                ctx->currentVisibility = ClangUML_visibility_public;
                break; 	
            case CX_CXXProtected: 	
                ctx->currentVisibility = ClangUML_visibility_protected;
                break;
            case CX_CXXPrivate:
                ctx->currentVisibility = ClangUML_visibility_private;
                break;
            }
            ret = CXChildVisit_Recurse;
        }
        break;
    case CXCursor_FieldDecl:
#if DEBUG_AST      
        fprintf(stderr,"FieldDecl %s\n", clang_getCString(cursorName));
        {
            CXType t = clang_getCursorType(cursor);
            fprintf(stderr,"FieldDecl type %d = %s\n", (int)t.kind, clang_getTypeSpelling(t));
        }
#endif
        if(ctx->clangUML->field)
        {
            CXString fieldType = clang_getTypeSpelling(clang_getCursorType(cursor));
            ctx->clangUML->field(ctx->user_data, cursor, clang_getCString(fieldType), ctx->currentVisibility, ClangUML_classifier_normal);
            clang_disposeString(fieldType);
        }
        {
            CXType t = clang_getCursorType(cursor);
            if ( t.kind == CXType_Record)
            {
                CXString displayName = clang_getTypeSpelling(clang_getCursorType(cursor));
                ASTCursorRelationship_add(ctx, clang_getCString(displayName), ClangUML_classRelation_composition);
                clang_disposeString(displayName);
            }
        }
        ret = CXChildVisit_Continue;
        break;
    case CXCursor_ConversionFunction:
    case CXCursor_FunctionDecl:
#if DEBUG_AST      
        fprintf(stderr,"ConversionFunction/FunctionDecl %s\n", clang_getCString(cursorName));
#endif
        ret = CXChildVisit_Recurse;
        break;
    case CXCursor_TypedefDecl:
#if DEBUG_AST      
        fprintf(stderr,"TypedefDecl %s\n", clang_getCString(cursorName));
#endif
        ret = CXChildVisit_Continue;
        break;
    case CXCursor_CompoundStmt:
#if DEBUG_AST      
        fprintf(stderr,"CompoundStmt\n");
#endif
        switch (parent.kind)
        {
        case CXCursor_FunctionDecl:
        case CXCursor_CXXMethod:
        case CXCursor_Constructor:
        case CXCursor_Destructor:
        case CXCursor_ConversionFunction:
        case CXCursor_FunctionTemplate:
        case CXCursor_Namespace:
        case CXCursor_ClassDecl:
        case CXCursor_ClassTemplate:
        case CXCursor_ClassTemplatePartialSpecialization:
            cursor = parent;
            break;
        }
        ret = CXChildVisit_Recurse;
        break;
    case CXCursor_VarDecl:
#if DEBUG_AST      
        fprintf(stderr,"VarDecl\n");
#endif
        ret = CXChildVisit_Continue;
        break;
    case CXCursor_ParmDecl:
#if DEBUG_AST      
        fprintf(stderr,"ParmDecl %s\n", clang_getCString(cursorName));
#endif
        ret = CXChildVisit_Continue;
        break;
    case CXCursor_TypeRef:
#if DEBUG_AST      
        fprintf(stderr,"TypeRef\n");
#endif
        ret = CXChildVisit_Continue;
        break;
    case CXCursor_MemberRefExpr:
    case CXCursor_MemberRef:
    case CXCursor_CallExpr:
#if DEBUG_AST      
        fprintf(stderr,"MemberRefExpr/MemberRef/CallExpr\n");
#endif
        ret = CXChildVisit_Recurse;
        break;
    case CXCursor_CXXMethod:
    case CXCursor_Constructor:
    case CXCursor_Destructor:
    case CXCursor_FunctionTemplate:
#if DEBUG_AST      
        fprintf(stderr,"CXXMethod/Constructor/Destructor/FunctionTemplate\n");
#endif
        if(ctx->clangUML->methodEnter)
        {
            ctx->clangUML->methodEnter(ctx->user_data, cursor, ClangUML_visibility_public, ClangUML_classifier_normal);
        }
        clang_visitChildren(cursor,ASTVisitor, client_data);
        if(ctx->clangUML->methodLeave)
        {
            ctx->clangUML->methodLeave(ctx->user_data, cursor);
        }
        ret = CXChildVisit_Continue;
        //        ret = CXChildVisit_Recurse;
        break;
    case CXCursor_CXXBaseSpecifier:
#if DEBUG_AST      
        fprintf(stderr,"CXXBaseSpecifier '%s'\n", clang_getCString(cursorName));
#endif
        ret = CXChildVisit_Recurse;

        CXCursor refCursor = clang_getCursorReferenced(cursor);
        CXString displayName = clang_getCursorDisplayName(refCursor);
        ASTCursorRelationship_add(ctx, clang_getCString(displayName), ClangUML_classRelation_extension);
        clang_disposeString(displayName);
        break;
    case CXCursor_UnexposedAttr:
        ret = CXChildVisit_Continue;
        break;
    default:
#if DEBUG_AST      
        fprintf(stderr,"Default %d\n", cursor.kind);
#endif
        ret = CXChildVisit_Recurse;
        break;
    }
    clang_disposeString(cursorName);
    return ret;
}

CLANGUML_API int ClangUML_visitTranslationUnit(ClangUML_t cu, CXTranslationUnit tu, void* user_data)
{
    struct ASTVisitorCtx ctx;
    ctx.clangUML = cu;
    ctx.user_data = user_data;
    ctx.currentVisibility = ClangUML_visibility_public;
    ctx.relationships = NULL;
    clang_visitChildren(clang_getTranslationUnitCursor(tu), ASTVisitor, &ctx);
    return 0;
}
