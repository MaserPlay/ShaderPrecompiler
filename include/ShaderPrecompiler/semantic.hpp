#pragma once

#include <vector>
#include <string>

#include "ast.hpp"

namespace shader_precompiler {
	class SemanticVisitor : public shader_precompiler::ast::VisitorBase, public ast::BaseAstProcessor {
	public:
		struct Variable {
			std::string type{};
			std::string name{};
			Variable() = default;
			Variable(std::string type, std::string name) : type(type), name(name) {}
		};
		struct Func {
			std::string returnType{};
			std::string name{};
			std::vector<Variable> params{};
			Func() = default;
			Func(std::string returnType, std::string name, std::vector<Variable> params) : returnType(returnType), name(name), params(params) {}
		};
		struct Type {
			std::string name{};
			std::vector<Func> functions{};
			std::vector<Variable> variables{};
			Type() = default;
			Type(std::string name, std::vector<Variable> variables = {}, std::vector<Func> functions = {}) : functions(functions), variables(variables), name(name) {}
		};
	private:
		shader_precompiler::ast::BaseAstProcessor& from;
		IDiagnosticReporter& reporter;
		std::vector<Func> functions{};
		std::vector<Type> types{};
		std::vector<Variable>* currentVariables{};

		PRINT_ERROR_DEFINE(shader_precompiler::Error::Stage::SEMANTIC_AMALIZITER)


		inline bool isType(std::string s) {
			for (auto& var : types)
			{
				if (var.name == s) {
					return true;
				}
			}
			return false;
		}

		inline bool isFunctionName(std::string s) {
			for (auto& var : functions)
			{
				if (var.name == s) {
					return true;
				}
			}
			return false;
		}

		inline bool isVariableName(std::string s) {
			for (auto& var : *currentVariables)
			{
				if (var.name == s) {
					return true;
				}
			}
			return false;
		}

		inline auto addVariable(Variable& v) {
			(*currentVariables).push_back(v);
		}

		void visit(shader_precompiler::ast::nodes::CodeBlock& node) override;
		void visit(shader_precompiler::ast::nodes::Identifier& node) override;
		void visit(shader_precompiler::ast::nodes::Return& node) override;
		void visit(shader_precompiler::ast::nodes::Attribute& node) override;
		void visit(shader_precompiler::ast::nodes::VariableInitialization& node) override;
		void visit(shader_precompiler::ast::nodes::IfElse& node) override;
		void visit(shader_precompiler::ast::nodes::Operator& node) override;
		void visit(shader_precompiler::ast::nodes::FuncDeclaration& node) override;
		void visit(shader_precompiler::ast::nodes::FuncCall& node) override;
		void visit(shader_precompiler::ast::nodes::Func& node) override;
		void visit(shader_precompiler::ast::nodes::NumberExpr& node) override;
	public:
		SemanticVisitor(shader_precompiler::ast::BaseAstProcessor& from, IDiagnosticReporter& reporter) : from(from), reporter(reporter) {};
		shader_precompiler::ast::TreeResult processTree() override;

		template<typename... Args>
		inline void addTypes(Args&&... args) {
			static_assert(
				((std::is_convertible_v<Args, Type>) && ...),
				"All arguments must be Type"
				);

			(types.push_back(args), ...);
		}

		template<typename... Args>
		inline auto addFunctions(Args&&... args) {
			static_assert(
				((std::is_convertible_v<Args, Func>) && ...),
				"All arguments must be Func"
				);

			(functions.push_back(args), ...);
		}

		inline void addShaderTypesFunctions() {
            using T = shader_precompiler::SemanticVisitor::Type;
            using V = shader_precompiler::SemanticVisitor::Variable;
            using F = shader_precompiler::SemanticVisitor::Func;

            // ---------------------- ТИПЫ ----------------------

            addTypes(
                T("void"),
                T("bool"),
                T("int"),
                T("uint"),
                T("float"),
                T("double"),

                // vec2
                T("vec2", { V("float","x"), V("float","y"),
                            V("float","r"), V("float","g"),
                            V("float","s"), V("float","t") }),

                // vec3
                T("vec3", { V("float","x"), V("float","y"), V("float","z"),
                            V("float","r"), V("float","g"), V("float","b"),
                            V("float","s"), V("float","t"), V("float","p") }),

                // vec4
                T("vec4", { V("float","x"), V("float","y"), V("float","z"), V("float","w"),
                            V("float","r"), V("float","g"), V("float","b"), V("float","a"),
                            V("float","s"), V("float","t"), V("float","p"), V("float","q") }),

                // int-векторы
                T("ivec2", { V("int","x"), V("int","y") }),
                T("ivec3", { V("int","x"), V("int","y"), V("int","z") }),
                T("ivec4", { V("int","x"), V("int","y"), V("int","z"), V("int","w") }),

                // uint-векторы
                T("uvec2", { V("uint","x"), V("uint","y") }),
                T("uvec3", { V("uint","x"), V("uint","y"), V("uint","z") }),
                T("uvec4", { V("uint","x"), V("uint","y"), V("uint","z"), V("uint","w") }),

                // bool-векторы
                T("bvec2", { V("bool","x"), V("bool","y") }),
                T("bvec3", { V("bool","x"), V("bool","y"), V("bool","z") }),
                T("bvec4", { V("bool","x"), V("bool","y"), V("bool","z"), V("bool","w") }),

                // матрицы
                T("mat2"), T("mat3"), T("mat4"),
                T("mat2x2"), T("mat2x3"), T("mat2x4"),
                T("mat3x2"), T("mat3x3"), T("mat3x4"),
                T("mat4x2"), T("mat4x3"), T("mat4x4"),

                // сэмплеры
                T("sampler1D"), T("sampler2D"), T("sampler3D"), T("samplerCube"),
                T("sampler1DShadow"), T("sampler2DShadow"), T("samplerCubeShadow"),
                T("sampler1DArray"), T("sampler2DArray"),
                T("sampler1DArrayShadow"), T("sampler2DArrayShadow"),
                T("samplerCubeArray"), T("samplerCubeArrayShadow"),
                T("samplerBuffer"),
                T("sampler2DMS"), T("sampler2DMSArray"),
                T("sampler2DRect"), T("sampler2DRectShadow"),

                T("isampler1D"), T("isampler2D"), T("isampler3D"), T("isamplerCube"),
                T("isampler1DArray"), T("isampler2DArray"), T("isamplerCubeArray"),
                T("isampler2DMS"), T("isampler2DMSArray"), T("isamplerBuffer"), T("isampler2DRect"),

                T("usampler1D"), T("usampler2D"), T("usampler3D"), T("usamplerCube"),
                T("usampler1DArray"), T("usampler2DArray"), T("usamplerCubeArray"),
                T("usampler2DMS"), T("usampler2DMSArray"), T("usamplerBuffer"), T("usampler2DRect"),

                // image-типы (compute/SSBO shaders)
                T("image1D"), T("image2D"), T("image3D"), T("imageCube"),
                T("image1DArray"), T("image2DArray"), T("imageCubeArray"),
                T("imageBuffer"), T("image2DMS"), T("image2DMSArray"),

                T("atomic_uint")
            );

            // ---------------------- ФУНКЦИИ ----------------------

            addFunctions(
                // --- конструкторы ---
                F("vec2", "vec2", {}), F("vec3", "vec3", {}), F("vec4", "vec4", {}),
                F("ivec2", "ivec2", {}), F("ivec3", "ivec3", {}), F("ivec4", "ivec4", {}),
                F("uvec2", "uvec2", {}), F("uvec3", "uvec3", {}), F("uvec4", "uvec4", {}),
                F("bvec2", "bvec2", {}), F("bvec3", "bvec3", {}), F("bvec4", "bvec4", {}),
                F("mat2", "mat2", {}), F("mat3", "mat3", {}), F("mat4", "mat4", {}),
                F("mat2x3", "mat2x3", {}), F("mat2x4", "mat2x4", {}),
                F("mat3x2", "mat3x2", {}), F("mat3x4", "mat3x4", {}),
                F("mat4x2", "mat4x2", {}), F("mat4x3", "mat4x3", {}),
                F("float", "float", {}), F("int", "int", {}), F("uint", "uint", {}),
                F("bool", "bool", {}), F("double", "double", {}),

                // --- тригонометрия ---
                F("float", "radians", {}), F("float", "degrees", {}),
                F("float", "sin", {}), F("float", "cos", {}), F("float", "tan", {}),
                F("float", "asin", {}), F("float", "acos", {}), F("float", "atan", {}),
                F("float", "sinh", {}), F("float", "cosh", {}), F("float", "tanh", {}),
                F("float", "asinh", {}), F("float", "acosh", {}), F("float", "atanh", {}),

                // --- экспоненциальные ---
                F("float", "pow", {}), F("float", "exp", {}), F("float", "log", {}),
                F("float", "exp2", {}), F("float", "log2", {}), F("float", "sqrt", {}),
                F("float", "inversesqrt", {}),

                // --- общие ---
                F("float", "abs", {}), F("float", "sign", {}),
                F("float", "floor", {}), F("float", "trunc", {}), F("float", "round", {}),
                F("float", "roundEven", {}), F("float", "ceil", {}), F("float", "fract", {}),
                F("float", "mod", {}), F("float", "modf", {}),
                F("float", "min", {}), F("float", "max", {}), F("float", "clamp", {}),
                F("float", "mix", {}), F("float", "step", {}), F("float", "smoothstep", {}),
                F("bool", "isnan", {}), F("bool", "isinf", {}),
                F("int", "floatBitsToInt", {}), F("uint", "floatBitsToUint", {}),
                F("float", "intBitsToFloat", {}), F("float", "uintBitsToFloat", {}),
                F("float", "fma", {}), F("float", "frexp", {}), F("float", "ldexp", {}),

                // --- упаковка/распаковка ---
                F("uint", "packUnorm2x16", {}), F("uint", "packSnorm2x16", {}),
                F("uint", "packUnorm4x8", {}), F("uint", "packSnorm4x8", {}),
                F("vec2", "unpackUnorm2x16", {}), F("vec2", "unpackSnorm2x16", {}),
                F("vec4", "unpackUnorm4x8", {}), F("vec4", "unpackSnorm4x8", {}),
                F("uint", "packHalf2x16", {}), F("vec2", "unpackHalf2x16", {}),
                F("double", "packDouble2x32", {}), F("uvec2", "unpackDouble2x32", {}),

                // --- геометрические ---
                F("float", "length", {}), F("float", "distance", {}), F("float", "dot", {}),
                F("vec3", "cross", {}), F("vec3", "normalize", {}),
                F("vec3", "faceforward", {}), F("vec3", "reflect", {}), F("vec3", "refract", {}),

                // --- матричные ---
                F("mat4", "matrixCompMult", {}), F("mat4", "outerProduct", {}),
                F("mat4", "transpose", {}), F("float", "determinant", {}), F("mat4", "inverse", {}),

                // --- векторные отношения ---
                F("bvec4", "lessThan", {}), F("bvec4", "lessThanEqual", {}),
                F("bvec4", "greaterThan", {}), F("bvec4", "greaterThanEqual", {}),
                F("bvec4", "equal", {}), F("bvec4", "notEqual", {}),
                F("bool", "any", {}), F("bool", "all", {}), F("bvec4", "not", {}),

                // --- целочисленные ---
                F("uint", "uaddCarry", {}), F("uint", "usubBorrow", {}),
                F("void", "umulExtended", {}), F("void", "imulExtended", {}),
                F("int", "bitfieldExtract", {}), F("int", "bitfieldInsert", {}),
                F("int", "bitfieldReverse", {}), F("int", "bitCount", {}),
                F("int", "findLSB", {}), F("int", "findMSB", {}),

                // --- текстуры ---
                F("ivec2", "textureSize", {}), F("float", "textureQueryLod", {}),
                F("int", "textureQueryLevels", {}),
                F("vec4", "texture", {}), F("vec4", "textureProj", {}),
                F("vec4", "textureLod", {}), F("vec4", "textureOffset", {}),
                F("vec4", "texelFetch", {}), F("vec4", "texelFetchOffset", {}),
                F("vec4", "textureProjOffset", {}), F("vec4", "textureLodOffset", {}),
                F("vec4", "textureProjLod", {}), F("vec4", "textureProjLodOffset", {}),
                F("vec4", "textureGrad", {}), F("vec4", "textureGradOffset", {}),
                F("vec4", "textureProjGrad", {}), F("vec4", "textureProjGradOffset", {}),
                F("vec4", "textureGather", {}), F("vec4", "textureGatherOffset", {}),
                F("vec4", "textureGatherOffsets", {}),

                // --- производные фрагмента ---
                F("float", "dFdx", {}), F("float", "dFdy", {}),
                F("float", "dFdxCoarse", {}), F("float", "dFdyCoarse", {}),
                F("float", "dFdxFine", {}), F("float", "dFdyFine", {}),
                F("float", "fwidth", {}), F("float", "fwidthCoarse", {}), F("float", "fwidthFine", {}),
                F("float", "interpolateAtCentroid", {}),
                F("float", "interpolateAtSample", {}),
                F("float", "interpolateAtOffset", {}),

                // --- шум (deprecated, но встречается) ---
                F("float", "noise1", {}), F("vec2", "noise2", {}),
                F("vec3", "noise3", {}), F("vec4", "noise4", {}),

                // --- геометрический шейдер ---
                F("void", "EmitVertex", {}), F("void", "EndPrimitive", {}),
                F("void", "EmitStreamVertex", {}), F("void", "EndStreamPrimitive", {}),

                // --- барьеры/синхронизация (compute) ---
                F("void", "barrier", {}),
                F("void", "memoryBarrier", {}), F("void", "memoryBarrierAtomicCounter", {}),
                F("void", "memoryBarrierBuffer", {}), F("void", "memoryBarrierShared", {}),
                F("void", "memoryBarrierImage", {}), F("void", "groupMemoryBarrier", {}),

                // --- atomic-счётчики ---
                F("uint", "atomicCounter", {}), F("uint", "atomicCounterIncrement", {}),
                F("uint", "atomicCounterDecrement", {}),
                F("uint", "atomicAdd", {}), F("uint", "atomicMin", {}), F("uint", "atomicMax", {}),
                F("uint", "atomicAnd", {}), F("uint", "atomicOr", {}), F("uint", "atomicXor", {}),
                F("uint", "atomicExchange", {}), F("uint", "atomicCompSwap", {})
            );
		}
	};
}