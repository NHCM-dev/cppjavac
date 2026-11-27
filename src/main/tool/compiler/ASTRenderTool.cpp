//
// Created by NHCM on 2026/11/27.
//

#include "ASTRenderTool.h"

#include <charconv>
#include <type_traits>

#include "RenderText.h"
#include "main/parser/ast/astdefs/Expressions.h"
#include "main/tool/EncodingTool.h"

namespace
{
    using namespace javac;

    struct AstTree
    {
        std::string label;
        std::vector<AstTree> children;
        std::optional<SourceRange> range = std::nullopt;

        template<typename T>
        void add(std::string_view role, const T& value);

        template<typename T>
        void add(std::string_view role, const Uptr<T>& value);

        template<typename T>
        void add(std::string_view role, const std::optional<T>& value);

        template<typename T>
        void add(std::string_view role, const std::vector<T>& values, bool showEmpty = false);
    };

    AstTree makeTree(const ASTNode& node, std::string label)
    {
        return {std::move(label), {}, node.range};
    }

    void renderChildren(const AstTree& tree, const std::string& indent, std::string& output)
    {
        for (std::size_t i = 0; i < tree.children.size(); ++i)
        {
            const auto& child = tree.children[i];
            const bool last = i + 1 == tree.children.size();
            output += indent;
            output += last ? "└── " : "├── ";
            output += child.label;
            if (child.range)
            {
                output += ' ' + render::rangeText(*child.range);
            }
            output += '\n';
            renderChildren(child, indent + (last ? "    " : "│   "), output);
        }
    }

    std::string_view enumName(FlagType value)
    {
        switch (value)
        {
            case FlagType::Public: return "public";
            case FlagType::Protected: return "protected";
            case FlagType::Private: return "private";
            case FlagType::Abstract: return "abstract";
            case FlagType::Static: return "static";
            case FlagType::Final: return "final";
            case FlagType::Transient: return "transient";
            case FlagType::Volatile: return "volatile";
            case FlagType::Synchronized: return "synchronized";
            case FlagType::Native: return "native";
            case FlagType::Strictfp: return "strictfp";
            case FlagType::Default: return "default";
            case FlagType::Sealed: return "sealed";
            case FlagType::NonSealed: return "non-sealed";
        }
        return "<unknown>";
    }

    std::string_view enumName(GenericType value)
    {
        switch (value)
        {
            case GenericType::Exact: return "Exact";
            case GenericType::Unbounded: return "Unbounded";
            case GenericType::Extends: return "Extends";
            case GenericType::Super: return "Super";
        }
        return "<unknown>";
    }

    std::string_view enumName(ObjectType value)
    {
        switch (value)
        {
            case ObjectType::Void: return "Void";
            case ObjectType::Primitive: return "Primitive";
            case ObjectType::Class: return "Class";
            case ObjectType::Array: return "Array";
            case ObjectType::GenericClass: return "GenericClass";
            case ObjectType::Inferred: return "Inferred";
        }
        return "<unknown>";
    }

    std::string_view enumName(PrimitiveType value)
    {
        switch (value)
        {
            case PrimitiveType::Boolean: return "boolean";
            case PrimitiveType::Byte: return "byte";
            case PrimitiveType::Short: return "short";
            case PrimitiveType::Char: return "char";
            case PrimitiveType::Int: return "int";
            case PrimitiveType::Long: return "long";
            case PrimitiveType::Float: return "float";
            case PrimitiveType::Double: return "double";
        }
        return "<unknown>";
    }

    std::string_view enumName(ImportType value)
    {
        switch (value)
        {
            case ImportType::Type: return "Type";
            case ImportType::TypeOnDemand: return "TypeOnDemand";
            case ImportType::Static: return "Static";
            case ImportType::StaticOnDemand: return "StaticOnDemand";
            case ImportType::Module: return "Module";
        }
        return "<unknown>";
    }

    std::string_view enumName(ModuleDirectiveType value)
    {
        switch (value)
        {
            case ModuleDirectiveType::Requires: return "Requires";
            case ModuleDirectiveType::Exports: return "Exports";
            case ModuleDirectiveType::Opens: return "Opens";
            case ModuleDirectiveType::Uses: return "Uses";
            case ModuleDirectiveType::Provides: return "Provides";
        }
        return "<unknown>";
    }

    std::string_view enumName(CompilationUnitType value)
    {
        switch (value)
        {
            case CompilationUnitType::Ordinary: return "Ordinary";
            case CompilationUnitType::Modular: return "Modular";
            case CompilationUnitType::Compact: return "Compact";
        }
        return "<unknown>";
    }

    std::string_view enumName(ArithmeticOp value)
    {
        switch (value)
        {
            case ArithmeticOp::Add: return "+";
            case ArithmeticOp::Minus: return "-";
            case ArithmeticOp::Multiply: return "*";
            case ArithmeticOp::Divide: return "/";
            case ArithmeticOp::Modulo: return "%";
        }
        return "<unknown>";
    }

    std::string_view enumName(IncDecOp value)
    {
        switch (value)
        {
            case IncDecOp::PlusPlus: return "++";
            case IncDecOp::MinusMinus: return "--";
        }
        return "<unknown>";
    }

    std::string_view enumName(AssignOp value)
    {
        switch (value)
        {
            case AssignOp::Assign: return "=";
            case AssignOp::PlusAssign: return "+=";
            case AssignOp::MinusAssign: return "-=";
            case AssignOp::StarAssign: return "*=";
            case AssignOp::SlashAssign: return "/=";
            case AssignOp::PercentAssign: return "%=";
            case AssignOp::AmpersandAssign: return "&=";
            case AssignOp::PipeAssign: return "|=";
            case AssignOp::CaretAssign: return "^=";
            case AssignOp::LeftShiftAssign: return "<<=";
            case AssignOp::RightShiftAssign: return ">>=";
            case AssignOp::UnsignedRightShiftAssign: return ">>>=";
        }
        return "<unknown>";
    }

    std::string_view enumName(CompareOp value)
    {
        switch (value)
        {
            case CompareOp::Equal: return "==";
            case CompareOp::NotEqual: return "!=";
            case CompareOp::Less: return "<";
            case CompareOp::LessEqual: return "<=";
            case CompareOp::Greater: return ">";
            case CompareOp::GreaterEqual: return ">=";
        }
        return "<unknown>";
    }

    std::string_view enumName(LogicalOp value)
    {
        switch (value)
        {
            case LogicalOp::And: return "&&";
            case LogicalOp::Or: return "||";
        }
        return "<unknown>";
    }

    std::string_view enumName(BitwiseOp value)
    {
        switch (value)
        {
            case BitwiseOp::And: return "&";
            case BitwiseOp::Or: return "|";
            case BitwiseOp::Xor: return "^";
        }
        return "<unknown>";
    }

    std::string_view enumName(ShiftOp value)
    {
        switch (value)
        {
            case ShiftOp::Left: return "<<";
            case ShiftOp::Right: return ">>";
            case ShiftOp::UnsignedRight: return ">>>";
        }
        return "<unknown>";
    }

    std::string_view enumName(UnaryOp value)
    {
        switch (value)
        {
            case UnaryOp::Plus: return "+";
            case UnaryOp::Minus: return "-";
            case UnaryOp::Not: return "!";
            case UnaryOp::Complement: return "~";
        }
        return "<unknown>";
    }

    void addVariableChildren(AstTree& tree, const VariableDecl& node)
    {
        tree.add("Modifiers", node.modifiers);
        tree.add("Type", node.type);
        tree.add("Declarators", node.declarators);
    }

    void addOperands(AstTree& tree, const BinaryExpr& node)
    {
        tree.add("Left", node.left);
        tree.add("Right", node.right);
    }

    AstTree nodeTree(const UnnamedPattern& node)
    {
        return makeTree(node, "Unnamed pattern");
    }

    AstTree nodeTree(const EmptyDecl& node)
    {
        return makeTree(node, "Empty declaration");
    }

    AstTree nodeTree(const FieldDecl& node)
    {
        auto tree = makeTree(node, "Field declaration");
        addVariableChildren(tree, node);
        return tree;
    }

    AstTree nodeTree(const AnnotationDecl& node)
    {
        auto tree = makeTree(node, "Annotation declaration  " + EncodingTool::utf16ToUtf8(node.name));
        tree.add("Modifiers", node.modifiers);
        tree.add("Members", node.members);
        return tree;
    }

    AstTree nodeTree(const EmptyStatement& node)
    {
        return makeTree(node, "Empty statement");
    }

    AstTree nodeTree(const NullExpr& node)
    {
        return makeTree(node, "Null literal");
    }

    AstTree nodeTree(const AnnotationArg& node)
    {
        auto tree = makeTree(node, "Annotation argument");
        tree.add("Name", node.name);
        tree.add("Value", node.value);
        return tree;
    }

    AstTree nodeTree(const Annotation& node)
    {
        auto tree = makeTree(node, "Annotation  @" + EncodingTool::utf16ToUtf8(node.name));
        tree.add("Arguments", node.arguments);
        return tree;
    }

    AstTree nodeTree(const ModifierFlag& node)
    {
        auto tree = makeTree(node, "Modifier");
        tree.add("Keyword", node.flagType);
        tree.add("Annotation", node.annotation);
        return tree;
    }

    AstTree nodeTree(const TypeArg& node)
    {
        auto tree = makeTree(node, "Type argument");
        tree.add("Kind", node.genericType);
        tree.add("Annotations", node.annotations);
        tree.add("Type", node.type);
        return tree;
    }

    AstTree nodeTree(const TypeNamePart& node)
    {
        auto tree = makeTree(node, "Type name part  " + EncodingTool::utf16ToUtf8(node.name));
        tree.add("Annotations", node.annotations);
        tree.add("Type arguments", node.typeArguments);
        tree.add("Diamond", node.diamond);
        return tree;
    }

    AstTree nodeTree(const ArrayDimension& node)
    {
        auto tree = makeTree(node, "Array dimension");
        tree.add("Annotations", node.annotations);
        return tree;
    }

    AstTree nodeTree(const TypeNode& node)
    {
        auto tree = makeTree(node, "Type");
        tree.add("Kind", node.objectType);
        tree.add("Annotations", node.annotations);
        tree.add("Primitive type", node.primitiveType);
        tree.add("Name parts", node.nameParts);
        tree.add("Element type", node.elementType);
        tree.add("Dimensions", node.dimensions);
        return tree;
    }

    AstTree nodeTree(const GenericParameter& node)
    {
        auto tree = makeTree(node, "Type parameter  " + EncodingTool::utf16ToUtf8(node.name));
        tree.add("Annotations", node.annotations);
        tree.add("Bounds", node.bounds);
        return tree;
    }

    AstTree nodeTree(const VariableDeclarator& node)
    {
        auto tree = makeTree(node, "Variable  " + EncodingTool::utf16ToUtf8(node.name));
        tree.add("Dimensions", node.dimensions);
        tree.add("Initializer", node.initializer);
        return tree;
    }

    AstTree nodeTree(const VariableDecl& node)
    {
        auto tree = makeTree(node, "Variable declaration");
        addVariableChildren(tree, node);
        return tree;
    }

    AstTree nodeTree(const TypePattern& node)
    {
        auto tree = makeTree(node, "Type pattern  " + EncodingTool::utf16ToUtf8(node.name));
        tree.add("Modifiers", node.modifiers);
        tree.add("Type", node.type);
        return tree;
    }

    AstTree nodeTree(const RecordPattern& node)
    {
        auto tree = makeTree(node, "Record pattern");
        tree.add("Type", node.type);
        tree.add("Components", node.components);
        return tree;
    }

    AstTree nodeTree(const PackageDecl& node)
    {
        auto tree = makeTree(node, "Package  " + EncodingTool::utf16ToUtf8(node.name));
        tree.add("Annotations", node.annotations);
        return tree;
    }

    AstTree nodeTree(const ImportDecl& node)
    {
        auto tree = makeTree(node, "Import  " + EncodingTool::utf16ToUtf8(node.name));
        tree.add("Kind", node.importType);
        return tree;
    }

    AstTree nodeTree(const ModuleDirective& node)
    {
        auto tree = makeTree(node, "Module directive  " + EncodingTool::utf16ToUtf8(node.name));
        tree.add("Kind", node.directiveType);
        tree.add("Target names", node.targetNames);
        tree.add("Static", node.isStatic);
        tree.add("Transitive", node.isTransitive);
        return tree;
    }

    AstTree nodeTree(const ModuleDecl& node)
    {
        auto tree = makeTree(node, "Module  " + EncodingTool::utf16ToUtf8(node.name));
        tree.add("Annotations", node.annotations);
        tree.add("Open", node.isOpen);
        tree.add("Directives", node.directives);
        return tree;
    }

    AstTree nodeTree(const CompilationUnit& node)
    {
        auto tree = makeTree(node, "Compilation unit");
        tree.add("Kind", node.unitType);
        tree.add("Package", node.packageDecl);
        tree.add("Imports", node.importDecls);
        tree.add("Declarations", node.decls);
        tree.add("Module", node.moduleDecl);
        return tree;
    }

    AstTree nodeTree(const ParameterDecl& node)
    {
        auto tree = makeTree(node, "Parameter  " + EncodingTool::utf16ToUtf8(node.name));
        tree.add("Modifiers", node.modifiers);
        tree.add("Type", node.type);
        tree.add("Dimensions", node.dimensions);
        tree.add("Varargs", node.varargs);
        tree.add("Default value", node.defaultValue);
        return tree;
    }

    AstTree nodeTree(const ReceiverParameter& node)
    {
        auto tree = makeTree(node, "Receiver parameter");
        tree.add("Type", node.type);
        tree.add("Qualifier", node.qualifier);
        return tree;
    }

    AstTree nodeTree(const MethodDecl& node)
    {
        auto tree = makeTree(node, "Method  " + EncodingTool::utf16ToUtf8(node.name));
        tree.add("Modifiers", node.modifiers);
        tree.add("Type parameters", node.typeParameters);
        tree.add("Returns", node.returnType);
        tree.add("Receiver", node.receiverParam);
        tree.add("Parameters", node.parameters);
        tree.add("Return dimensions", node.returnDimensions);
        tree.add("Throws", node.thrownTypes);
        tree.add("Default value", node.defaultValue);
        tree.add("Body", node.body);
        return tree;
    }

    AstTree nodeTree(const ConstructorDecl& node)
    {
        auto tree = makeTree(node, "Constructor  " + EncodingTool::utf16ToUtf8(node.name));
        tree.add("Modifiers", node.modifiers);
        tree.add("Compact", node.isCompact);
        tree.add("Type parameters", node.typeParameters);
        tree.add("Receiver", node.receiverParam);
        tree.add("Parameters", node.parameters);
        tree.add("Throws", node.thrownTypes);
        tree.add("Body", node.body);
        return tree;
    }

    AstTree nodeTree(const InitializerDecl& node)
    {
        auto tree = makeTree(node, node.isStatic ? "Static initializer" : "Instance initializer");
        tree.add("Body", node.body);
        return tree;
    }

    AstTree nodeTree(const TypeDecl& node)
    {
        auto tree = makeTree(node, "Type  " + EncodingTool::utf16ToUtf8(node.name));
        tree.add("Modifiers", node.modifiers);
        tree.add("Members", node.members);
        return tree;
    }

    AstTree nodeTree(const ClassDecl& node)
    {
        auto tree = makeTree(node, "Class  " + EncodingTool::utf16ToUtf8(node.name));
        tree.add("Modifiers", node.modifiers);
        tree.add("Type parameters", node.typeParameters);
        tree.add("Extends", node.superClass);
        tree.add("Implements", node.interfaces);
        tree.add("Permits", node.permittedTypes);
        tree.add("Members", node.members);
        return tree;
    }

    AstTree nodeTree(const InterfaceDecl& node)
    {
        auto tree = makeTree(node, "Interface  " + EncodingTool::utf16ToUtf8(node.name));
        tree.add("Modifiers", node.modifiers);
        tree.add("Type parameters", node.typeParameters);
        tree.add("Extends", node.superInterfaces);
        tree.add("Permits", node.permittedTypes);
        tree.add("Members", node.members);
        return tree;
    }

    AstTree nodeTree(const EnumConstantDecl& node)
    {
        auto tree = makeTree(node, "Enum constant  " + EncodingTool::utf16ToUtf8(node.name));
        tree.add("Annotations", node.annotations);
        tree.add("Constructor args", node.constructorArgs);
        if (node.members)
        {
            tree.add("Class body", *node.members, true);
        }
        return tree;
    }

    AstTree nodeTree(const EnumDecl& node)
    {
        auto tree = makeTree(node, "Enum  " + EncodingTool::utf16ToUtf8(node.name));
        tree.add("Modifiers", node.modifiers);
        tree.add("Constants", node.constants);
        tree.add("Implements", node.interfaces);
        tree.add("Members", node.members);
        return tree;
    }

    AstTree nodeTree(const RecordComponent& node)
    {
        auto tree = makeTree(node, "Record component  " + EncodingTool::utf16ToUtf8(node.name));
        tree.add("Annotations", node.annotations);
        tree.add("Type", node.type);
        tree.add("Varargs", node.varargs);
        return tree;
    }

    AstTree nodeTree(const RecordDecl& node)
    {
        auto tree = makeTree(node, "Record  " + EncodingTool::utf16ToUtf8(node.name));
        tree.add("Modifiers", node.modifiers);
        tree.add("Type parameters", node.typeParameters);
        tree.add("Components", node.components);
        tree.add("Implements", node.interfaces);
        tree.add("Members", node.members);
        return tree;
    }

    AstTree nodeTree(const BlockStatement& node)
    {
        auto tree = makeTree(node, "Block");
        tree.add("Statements", node.statements);
        return tree;
    }

    AstTree nodeTree(const DeclStatement& node)
    {
        auto tree = makeTree(node, "Declaration statement");
        tree.add("Declaration", node.declaration);
        return tree;
    }

    AstTree nodeTree(const ExprStatement& node)
    {
        auto tree = makeTree(node, "Expression statement");
        tree.add("Expression", node.expression);
        return tree;
    }

    AstTree nodeTree(const ConstructorCallStatement& node)
    {
        auto tree = makeTree(node, "Constructor invocation");
        tree.add("Super", node.isSuper);
        tree.add("Qualifier", node.qualifier);
        tree.add("Type arguments", node.typeArguments);
        tree.add("Arguments", node.arguments);
        return tree;
    }

    AstTree nodeTree(const ReturnStatement& node)
    {
        auto tree = makeTree(node, "Return");
        tree.add("Value", node.value);
        return tree;
    }

    AstTree nodeTree(const IfStatement& node)
    {
        auto tree = makeTree(node, "If");
        tree.add("Condition", node.condition);
        tree.add("Then branch", node.thenBranch);
        tree.add("Else branch", node.elseBranch);
        return tree;
    }

    AstTree nodeTree(const WhileStatement& node)
    {
        auto tree = makeTree(node, "While");
        tree.add("Condition", node.condition);
        tree.add("Body", node.body);
        return tree;
    }

    AstTree nodeTree(const DoWhileStatement& node)
    {
        auto tree = makeTree(node, "Do-while");
        tree.add("Body", node.body);
        tree.add("Condition", node.condition);
        return tree;
    }

    AstTree nodeTree(const ForStatement& node)
    {
        auto tree = makeTree(node, "For");
        tree.add("Initializer declaration", node.initializerDecl);
        tree.add("Initializers", node.initializerExprs);
        tree.add("Condition", node.condition);
        tree.add("Updates", node.updateExprs);
        tree.add("Body", node.body);
        return tree;
    }

    AstTree nodeTree(const EnhancedForStatement& node)
    {
        auto tree = makeTree(node, "Enhanced for");
        tree.add("Variable decl", node.variableDecl);
        tree.add("Iterable", node.iterableExpr);
        tree.add("Body", node.body);
        return tree;
    }

    AstTree nodeTree(const LabeledStatement& node)
    {
        auto tree = makeTree(node, "Label  " + EncodingTool::utf16ToUtf8(node.label));
        tree.add("Statement", node.statement);
        return tree;
    }

    AstTree nodeTree(const BreakStatement& node)
    {
        auto tree = makeTree(node, "Break");
        tree.add("Label", node.label);
        return tree;
    }

    AstTree nodeTree(const ContinueStatement& node)
    {
        auto tree = makeTree(node, "Continue");
        tree.add("Label", node.label);
        return tree;
    }

    AstTree nodeTree(const GotoStatement& node)
    {
        return makeTree(node, "Goto  " + EncodingTool::utf16ToUtf8(node.label));
    }

    AstTree nodeTree(const AssertStatement& node)
    {
        auto tree = makeTree(node, "Assert");
        tree.add("Condition", node.condition);
        tree.add("Message", node.message);
        return tree;
    }

    AstTree nodeTree(const SynchronizedStatement& node)
    {
        auto tree = makeTree(node, "Synchronized");
        tree.add("Monitor", node.monitorExpr);
        tree.add("Body", node.body);
        return tree;
    }

    AstTree nodeTree(const TryResource& node)
    {
        auto tree = makeTree(node, "Resource");
        tree.add("Declaration", node.resourceDecl);
        tree.add("Reference", node.referenceExpr);
        return tree;
    }

    AstTree nodeTree(const CatchClause& node)
    {
        auto tree = makeTree(node, "Catch  " + EncodingTool::utf16ToUtf8(node.name));
        tree.add("Modifiers", node.modifiers);
        tree.add("Exception types", node.exceptionTypes);
        tree.add("Body", node.body);
        return tree;
    }

    AstTree nodeTree(const TryStatement& node)
    {
        auto tree = makeTree(node, "Try");
        tree.add("Resources", node.resources);
        tree.add("Try body", node.tryBlock);
        tree.add("Catches", node.catches);
        tree.add("Finally", node.finallyBlock);
        return tree;
    }

    AstTree nodeTree(const ThrowStatement& node)
    {
        auto tree = makeTree(node, "Throw");
        tree.add("Value", node.value);
        return tree;
    }

    AstTree nodeTree(const SwitchLabel& node)
    {
        auto tree = makeTree(node, "Switch label");
        tree.add("Values", node.caseExprs);
        tree.add("Patterns", node.patterns);
        tree.add("Default", node.hasDefault);
        tree.add("Guard", node.guardExpr);
        return tree;
    }

    AstTree nodeTree(const SwitchCase& node)
    {
        auto tree = makeTree(node, "Switch case");
        tree.add("Labels", node.labels);
        tree.add("Statements", node.statements);
        tree.add("Rule expr", node.ruleExpr);
        tree.add("Rule statement", node.ruleStatement);
        return tree;
    }

    AstTree nodeTree(const SwitchStatement& node)
    {
        auto tree = makeTree(node, "Switch statement");
        tree.add("Selector", node.selectorExpr);
        tree.add("Cases", node.cases);
        return tree;
    }

    AstTree nodeTree(const YieldStatement& node)
    {
        auto tree = makeTree(node, "Yield");
        tree.add("Value", node.value);
        return tree;
    }

    AstTree nodeTree(const BinaryExpr& node)
    {
        auto tree = makeTree(node, "Binary");
        addOperands(tree, node);
        return tree;
    }

    AstTree nodeTree(const ArithmeticExpr& node)
    {
        auto tree = makeTree(node, "Arithmetic");
        tree.add("Operator", node.op);
        addOperands(tree, node);
        return tree;
    }

    AstTree nodeTree(const IncDecExpr& node)
    {
        auto tree = makeTree(node, "Increment or decrement");
        tree.add("Operator", node.op);
        tree.add("Operand", node.operand);
        tree.add("Postfix", node.isPostfix);
        return tree;
    }

    AstTree nodeTree(const AssignExpr& node)
    {
        auto tree = makeTree(node, "Assign");
        tree.add("Operator", node.op);
        addOperands(tree, node);
        return tree;
    }

    AstTree nodeTree(const CompareExpr& node)
    {
        auto tree = makeTree(node, "Compare");
        tree.add("Operator", node.op);
        addOperands(tree, node);
        return tree;
    }

    AstTree nodeTree(const LogicalExpr& node)
    {
        auto tree = makeTree(node, "Logical");
        tree.add("Operator", node.op);
        addOperands(tree, node);
        return tree;
    }

    AstTree nodeTree(const BitwiseExpr& node)
    {
        auto tree = makeTree(node, "Bitwise");
        tree.add("Operator", node.op);
        addOperands(tree, node);
        return tree;
    }

    AstTree nodeTree(const ShiftExpr& node)
    {
        auto tree = makeTree(node, "Shift");
        tree.add("Operator", node.op);
        addOperands(tree, node);
        return tree;
    }

    AstTree nodeTree(const UnaryExpr& node)
    {
        auto tree = makeTree(node, "Unary");
        tree.add("Operator", node.op);
        tree.add("Operand", node.operand);
        return tree;
    }

    AstTree nodeTree(const ConditionalExpr& node)
    {
        auto tree = makeTree(node, "Conditional");
        tree.add("Condition", node.condition);
        tree.add("When true", node.whenTrue);
        tree.add("When false", node.whenFalse);
        return tree;
    }

    AstTree nodeTree(const NameExpr& node)
    {
        return makeTree(node, "Name  " + EncodingTool::utf16ToUtf8(node.name));
    }

    AstTree nodeTree(const ParenExpr& node)
    {
        auto tree = makeTree(node, "Parenthesized expression");
        tree.add("Expression", node.expression);
        return tree;
    }

    AstTree nodeTree(const ThisExpr& node)
    {
        auto tree = makeTree(node, "This");
        tree.add("Qualifier", node.qualifier);
        return tree;
    }

    AstTree nodeTree(const SuperExpr& node)
    {
        auto tree = makeTree(node, "Super");
        tree.add("Qualifier", node.qualifier);
        return tree;
    }

    AstTree nodeTree(const FieldAccessExpr& node)
    {
        auto tree = makeTree(node, "Field access  " + EncodingTool::utf16ToUtf8(node.name));
        tree.add("Receiver", node.receiver);
        return tree;
    }

    AstTree nodeTree(const ArrayAccessExpr& node)
    {
        auto tree = makeTree(node, "Array access");
        tree.add("Array expr", node.arrayExpr);
        tree.add("Index expr", node.indexExpr);
        return tree;
    }

    AstTree nodeTree(const CallExpr& node)
    {
        auto tree = makeTree(node, "Call");
        tree.add("Callee", node.callee);
        tree.add("Type arguments", node.typeArguments);
        tree.add("Arguments", node.arguments);
        return tree;
    }

    AstTree nodeTree(const NewExpr& node)
    {
        auto tree = makeTree(node, "New object");
        tree.add("Qualifier", node.qualifier);
        tree.add("Type arguments", node.typeArguments);
        tree.add("Type", node.type);
        tree.add("Arguments", node.arguments);
        if (node.members)
        {
            tree.add("Class body", *node.members, true);
        }
        return tree;
    }

    AstTree nodeTree(const ArrayInitializerExpr& node)
    {
        auto tree = makeTree(node, "Array initializer");
        tree.add("Elements", node.elements);
        return tree;
    }

    AstTree nodeTree(const ArrayDimensionExpr& node)
    {
        auto tree = makeTree(node, "Array dimension");
        tree.add("Annotations", node.annotations);
        tree.add("Size", node.sizeExpr);
        return tree;
    }

    AstTree nodeTree(const NewArrayExpr& node)
    {
        auto tree = makeTree(node, "New array");
        tree.add("Element type", node.elementType);
        tree.add("Dimensions", node.dimensions);
        tree.add("Initializer", node.initializer);
        return tree;
    }

    AstTree nodeTree(const ClassLiteralExpr& node)
    {
        auto tree = makeTree(node, "Class literal");
        tree.add("Type", node.type);
        return tree;
    }

    AstTree nodeTree(const CastExpr& node)
    {
        auto tree = makeTree(node, "Cast");
        tree.add("Cast type", node.castType);
        tree.add("Additional bounds", node.additionalBounds);
        tree.add("Value", node.value);
        return tree;
    }

    AstTree nodeTree(const InstanceOfExpr& node)
    {
        auto tree = makeTree(node, "Instanceof");
        tree.add("Value", node.value);
        tree.add("Test type", node.testType);
        tree.add("Pattern", node.pattern);
        return tree;
    }

    AstTree nodeTree(const LambdaExpr& node)
    {
        auto tree = makeTree(node, "Lambda");
        tree.add("Parameters", node.parameters);
        tree.add("Expression body", node.exprBody);
        tree.add("Block body", node.blockBody);
        return tree;
    }

    AstTree nodeTree(const MethodRefExpr& node)
    {
        auto tree = makeTree(node, "Method reference");
        tree.add("Expr qualifier", node.exprQualifier);
        tree.add("Type qualifier", node.typeQualifier);
        tree.add("Type arguments", node.typeArguments);
        tree.add("Method name", node.methodName);
        return tree;
    }

    AstTree nodeTree(const SwitchExpr& node)
    {
        auto tree = makeTree(node, "Switch expression");
        tree.add("Selector", node.selectorExpr);
        tree.add("Cases", node.cases);
        return tree;
    }

    AstTree nodeTree(const IntExpr& node)
    {
        auto tree = makeTree(node, "Int literal");
        tree.add("Value", node.value);
        return tree;
    }

    AstTree nodeTree(const LongExpr& node)
    {
        auto tree = makeTree(node, "Long literal");
        tree.add("Value", node.value);
        return tree;
    }

    AstTree nodeTree(const FloatExpr& node)
    {
        auto tree = makeTree(node, "Float literal");
        tree.add("Value", node.value);
        return tree;
    }

    AstTree nodeTree(const DoubleExpr& node)
    {
        auto tree = makeTree(node, "Double literal");
        tree.add("Value", node.value);
        return tree;
    }

    AstTree nodeTree(const CharExpr& node)
    {
        auto tree = makeTree(node, "Char literal");
        tree.add("Value", node.value);
        return tree;
    }

    AstTree nodeTree(const ShortExpr& node)
    {
        auto tree = makeTree(node, "Short literal");
        tree.add("Value", node.value);
        return tree;
    }

    AstTree nodeTree(const ByteExpr& node)
    {
        auto tree = makeTree(node, "Byte literal");
        tree.add("Value", node.value);
        return tree;
    }

    AstTree nodeTree(const BooleanExpr& node)
    {
        auto tree = makeTree(node, "Boolean literal");
        tree.add("Value", node.value);
        return tree;
    }

    AstTree nodeTree(const StringExpr& node)
    {
        auto tree = makeTree(node, "String literal");
        tree.add("Value", node.value);
        tree.add("Text block", node.textBlock);
        return tree;
    }

    AstTree nodeTree(const Decl& node)
    {
        if (const auto* value = dynamic_cast<const FieldDecl*>(&node))
        {
            return nodeTree(*value);
        }
        if (const auto* value = dynamic_cast<const AnnotationDecl*>(&node))
        {
            return nodeTree(*value);
        }
        if (const auto* value = dynamic_cast<const ClassDecl*>(&node))
        {
            return nodeTree(*value);
        }
        if (const auto* value = dynamic_cast<const InterfaceDecl*>(&node))
        {
            return nodeTree(*value);
        }
        if (const auto* value = dynamic_cast<const EnumDecl*>(&node))
        {
            return nodeTree(*value);
        }
        if (const auto* value = dynamic_cast<const RecordDecl*>(&node))
        {
            return nodeTree(*value);
        }
        if (const auto* value = dynamic_cast<const EmptyDecl*>(&node))
        {
            return nodeTree(*value);
        }
        if (const auto* value = dynamic_cast<const VariableDecl*>(&node))
        {
            return nodeTree(*value);
        }
        if (const auto* value = dynamic_cast<const PackageDecl*>(&node))
        {
            return nodeTree(*value);
        }
        if (const auto* value = dynamic_cast<const ImportDecl*>(&node))
        {
            return nodeTree(*value);
        }
        if (const auto* value = dynamic_cast<const ModuleDecl*>(&node))
        {
            return nodeTree(*value);
        }
        if (const auto* value = dynamic_cast<const ParameterDecl*>(&node))
        {
            return nodeTree(*value);
        }
        if (const auto* value = dynamic_cast<const MethodDecl*>(&node))
        {
            return nodeTree(*value);
        }
        if (const auto* value = dynamic_cast<const ConstructorDecl*>(&node))
        {
            return nodeTree(*value);
        }
        if (const auto* value = dynamic_cast<const InitializerDecl*>(&node))
        {
            return nodeTree(*value);
        }
        if (const auto* value = dynamic_cast<const TypeDecl*>(&node))
        {
            return nodeTree(*value);
        }
        if (const auto* value = dynamic_cast<const EnumConstantDecl*>(&node))
        {
            return nodeTree(*value);
        }
        return makeTree(node, "Decl");
    }

    AstTree nodeTree(const Statement& node)
    {
        if (const auto* value = dynamic_cast<const EmptyStatement*>(&node))
        {
            return nodeTree(*value);
        }
        if (const auto* value = dynamic_cast<const BlockStatement*>(&node))
        {
            return nodeTree(*value);
        }
        if (const auto* value = dynamic_cast<const DeclStatement*>(&node))
        {
            return nodeTree(*value);
        }
        if (const auto* value = dynamic_cast<const ExprStatement*>(&node))
        {
            return nodeTree(*value);
        }
        if (const auto* value = dynamic_cast<const ConstructorCallStatement*>(&node))
        {
            return nodeTree(*value);
        }
        if (const auto* value = dynamic_cast<const ReturnStatement*>(&node))
        {
            return nodeTree(*value);
        }
        if (const auto* value = dynamic_cast<const IfStatement*>(&node))
        {
            return nodeTree(*value);
        }
        if (const auto* value = dynamic_cast<const WhileStatement*>(&node))
        {
            return nodeTree(*value);
        }
        if (const auto* value = dynamic_cast<const DoWhileStatement*>(&node))
        {
            return nodeTree(*value);
        }
        if (const auto* value = dynamic_cast<const ForStatement*>(&node))
        {
            return nodeTree(*value);
        }
        if (const auto* value = dynamic_cast<const EnhancedForStatement*>(&node))
        {
            return nodeTree(*value);
        }
        if (const auto* value = dynamic_cast<const LabeledStatement*>(&node))
        {
            return nodeTree(*value);
        }
        if (const auto* value = dynamic_cast<const BreakStatement*>(&node))
        {
            return nodeTree(*value);
        }
        if (const auto* value = dynamic_cast<const ContinueStatement*>(&node))
        {
            return nodeTree(*value);
        }
        if (const auto* value = dynamic_cast<const GotoStatement*>(&node))
        {
            return nodeTree(*value);
        }
        if (const auto* value = dynamic_cast<const AssertStatement*>(&node))
        {
            return nodeTree(*value);
        }
        if (const auto* value = dynamic_cast<const SynchronizedStatement*>(&node))
        {
            return nodeTree(*value);
        }
        if (const auto* value = dynamic_cast<const TryStatement*>(&node))
        {
            return nodeTree(*value);
        }
        if (const auto* value = dynamic_cast<const ThrowStatement*>(&node))
        {
            return nodeTree(*value);
        }
        if (const auto* value = dynamic_cast<const SwitchStatement*>(&node))
        {
            return nodeTree(*value);
        }
        if (const auto* value = dynamic_cast<const YieldStatement*>(&node))
        {
            return nodeTree(*value);
        }
        return makeTree(node, "Statement");
    }

    AstTree nodeTree(const Expr& node)
    {
        if (const auto* value = dynamic_cast<const ArithmeticExpr*>(&node))
        {
            return nodeTree(*value);
        }
        if (const auto* value = dynamic_cast<const AssignExpr*>(&node))
        {
            return nodeTree(*value);
        }
        if (const auto* value = dynamic_cast<const CompareExpr*>(&node))
        {
            return nodeTree(*value);
        }
        if (const auto* value = dynamic_cast<const LogicalExpr*>(&node))
        {
            return nodeTree(*value);
        }
        if (const auto* value = dynamic_cast<const BitwiseExpr*>(&node))
        {
            return nodeTree(*value);
        }
        if (const auto* value = dynamic_cast<const ShiftExpr*>(&node))
        {
            return nodeTree(*value);
        }
        if (const auto* value = dynamic_cast<const NullExpr*>(&node))
        {
            return nodeTree(*value);
        }
        if (const auto* value = dynamic_cast<const Annotation*>(&node))
        {
            return nodeTree(*value);
        }
        if (const auto* value = dynamic_cast<const BinaryExpr*>(&node))
        {
            return nodeTree(*value);
        }
        if (const auto* value = dynamic_cast<const IncDecExpr*>(&node))
        {
            return nodeTree(*value);
        }
        if (const auto* value = dynamic_cast<const UnaryExpr*>(&node))
        {
            return nodeTree(*value);
        }
        if (const auto* value = dynamic_cast<const ConditionalExpr*>(&node))
        {
            return nodeTree(*value);
        }
        if (const auto* value = dynamic_cast<const NameExpr*>(&node))
        {
            return nodeTree(*value);
        }
        if (const auto* value = dynamic_cast<const ParenExpr*>(&node))
        {
            return nodeTree(*value);
        }
        if (const auto* value = dynamic_cast<const ThisExpr*>(&node))
        {
            return nodeTree(*value);
        }
        if (const auto* value = dynamic_cast<const SuperExpr*>(&node))
        {
            return nodeTree(*value);
        }
        if (const auto* value = dynamic_cast<const FieldAccessExpr*>(&node))
        {
            return nodeTree(*value);
        }
        if (const auto* value = dynamic_cast<const ArrayAccessExpr*>(&node))
        {
            return nodeTree(*value);
        }
        if (const auto* value = dynamic_cast<const CallExpr*>(&node))
        {
            return nodeTree(*value);
        }
        if (const auto* value = dynamic_cast<const NewExpr*>(&node))
        {
            return nodeTree(*value);
        }
        if (const auto* value = dynamic_cast<const ArrayInitializerExpr*>(&node))
        {
            return nodeTree(*value);
        }
        if (const auto* value = dynamic_cast<const NewArrayExpr*>(&node))
        {
            return nodeTree(*value);
        }
        if (const auto* value = dynamic_cast<const ClassLiteralExpr*>(&node))
        {
            return nodeTree(*value);
        }
        if (const auto* value = dynamic_cast<const CastExpr*>(&node))
        {
            return nodeTree(*value);
        }
        if (const auto* value = dynamic_cast<const InstanceOfExpr*>(&node))
        {
            return nodeTree(*value);
        }
        if (const auto* value = dynamic_cast<const LambdaExpr*>(&node))
        {
            return nodeTree(*value);
        }
        if (const auto* value = dynamic_cast<const MethodRefExpr*>(&node))
        {
            return nodeTree(*value);
        }
        if (const auto* value = dynamic_cast<const SwitchExpr*>(&node))
        {
            return nodeTree(*value);
        }
        if (const auto* value = dynamic_cast<const IntExpr*>(&node))
        {
            return nodeTree(*value);
        }
        if (const auto* value = dynamic_cast<const LongExpr*>(&node))
        {
            return nodeTree(*value);
        }
        if (const auto* value = dynamic_cast<const FloatExpr*>(&node))
        {
            return nodeTree(*value);
        }
        if (const auto* value = dynamic_cast<const DoubleExpr*>(&node))
        {
            return nodeTree(*value);
        }
        if (const auto* value = dynamic_cast<const CharExpr*>(&node))
        {
            return nodeTree(*value);
        }
        if (const auto* value = dynamic_cast<const ShortExpr*>(&node))
        {
            return nodeTree(*value);
        }
        if (const auto* value = dynamic_cast<const ByteExpr*>(&node))
        {
            return nodeTree(*value);
        }
        if (const auto* value = dynamic_cast<const BooleanExpr*>(&node))
        {
            return nodeTree(*value);
        }
        if (const auto* value = dynamic_cast<const StringExpr*>(&node))
        {
            return nodeTree(*value);
        }
        return makeTree(node, "Expr");
    }

    AstTree nodeTree(const Pattern& node)
    {
        if (const auto* value = dynamic_cast<const UnnamedPattern*>(&node))
        {
            return nodeTree(*value);
        }
        if (const auto* value = dynamic_cast<const TypePattern*>(&node))
        {
            return nodeTree(*value);
        }
        if (const auto* value = dynamic_cast<const RecordPattern*>(&node))
        {
            return nodeTree(*value);
        }
        return makeTree(node, "Pattern");
    }

    template<typename T>
    void AstTree::add(std::string_view role, const T& value)
    {
        if constexpr (std::is_base_of_v<ASTNode, T>)
        {
            auto child = nodeTree(value);
            if (!role.empty() && child.label != role)
            {
                const std::string prefix = std::string(role) + "  ";
                if (!child.label.starts_with(prefix))
                {
                    child.label.insert(0, prefix);
                }
            }
            children.push_back(std::move(child));
        }
        else
        {
            std::string text;
            if constexpr (std::is_enum_v<T>)
            {
                text = enumName(value);
            }
            else if constexpr (std::is_same_v<T, std::u16string>)
            {
                text = render::quoteUtf16(value);
            }
            else if constexpr (std::is_same_v<T, char16_t>)
            {
                text = render::quoteUtf16(std::u16string_view(&value, 1), '\'');
            }
            else if constexpr (std::is_same_v<T, bool>)
            {
                text = value ? "true" : "false";
            }
            else if constexpr (std::is_floating_point_v<T>)
            {
                char buffer[64];
                const auto result = std::to_chars(buffer, buffer + sizeof(buffer), value);
                text.assign(buffer, result.ptr);
            }
            else
            {
                text = std::to_string(value);
            }
            if (!role.empty())
            {
                text = std::string(role) + "  " + text;
            }
            children.push_back({std::move(text), {}});
        }
    }

    template<typename T>
    void AstTree::add(std::string_view role, const Uptr<T>& value)
    {
        if (value)
        {
            add(role, *value);
        }
    }

    template<typename T>
    void AstTree::add(std::string_view role, const std::optional<T>& value)
    {
        if (value)
        {
            add(role, *value);
        }
    }

    template<typename T>
    void AstTree::add(std::string_view role, const std::vector<T>& values, bool showEmpty)
    {
        if (values.empty() && !showEmpty)
        {
            return;
        }
        AstTree group{std::string(role), {}};
        for (const auto& value : values)
        {
            group.add({}, value);
        }
        children.push_back(std::move(group));
    }
}

std::string ASTRenderTool::render(const Path& file, const CompilationUnit& unit)
{
    const auto tree = nodeTree(unit);
    std::string output = tree.label + "  " + file.filename().string();
    if (tree.range)
    {
        output += ' ' + render::rangeText(*tree.range);
    }
    output += '\n';
    renderChildren(tree, {}, output);
    return output;
}
