#include <lfortran/containers.h>
#include <lfortran/exception.h>
#include <lfortran/asr_utils.h>
#include <lfortran/asr_verify.h>


namespace LFortran {
namespace ASR {

class VerifyVisitor : public BaseWalkVisitor<VerifyVisitor>
{
private:
    SymbolTable *current_symtab;
    bool symbol_enter_ok = false;
public:

    // Requires the condition `cond` to be true. Raise an exception otherwise.
    void require(bool cond, const std::string &error_msg) {
        if (!cond) {
            throw LFortranException("ASR verify failed: " + error_msg);
        }
    }

    void visit_TranslationUnit(const TranslationUnit_t &x) {
        current_symtab = x.m_global_scope;
        require(x.m_global_scope != nullptr,
            "The TranslationUnit::m_global_scope cannot be nullptr");
        require(x.m_global_scope->parent == nullptr,
            "The TranslationUnit::m_global_scope->parent must be nullptr");
        for (auto &a : x.m_global_scope->scope) {
            symbol_enter_ok = true;
            this->visit_symbol(*a.second);
            symbol_enter_ok = false;
        }
        current_symtab = nullptr;
    }

    void visit_Program(const Program_t &x) {
        if (!symbol_enter_ok) return;
        SymbolTable *parent_symtab = current_symtab;
        current_symtab = x.m_symtab;
        require(x.m_symtab != nullptr,
            "The Program::m_symtab cannot be nullptr");
        require(x.m_symtab->parent == parent_symtab,
            "The Program::m_symtab->parent is not the right parent");
        for (auto &a : x.m_symtab->scope) {
            symbol_enter_ok = true;
            this->visit_symbol(*a.second);
            symbol_enter_ok = false;
        }
        symbol_enter_ok = false;
        for (size_t i=0; i<x.n_body; i++) {
            visit_stmt(*x.m_body[i]);
        }
        current_symtab = parent_symtab;
    }

    void visit_Module(const Module_t &x) {
        if (!symbol_enter_ok) return;
        SymbolTable *parent_symtab = current_symtab;
        current_symtab = x.m_symtab;
        require(x.m_symtab != nullptr,
            "The Module::m_symtab cannot be nullptr");
        require(x.m_symtab->parent == parent_symtab,
            "The Module::m_symtab->parent is not the right parent");
        for (auto &a : x.m_symtab->scope) {
            symbol_enter_ok = true;
            this->visit_symbol(*a.second);
            symbol_enter_ok = false;
        }
        symbol_enter_ok = false;
        current_symtab = parent_symtab;
    }

    void visit_Subroutine(const Subroutine_t &x) {
        if (!symbol_enter_ok) return;
        SymbolTable *parent_symtab = current_symtab;
        current_symtab = x.m_symtab;
        require(x.m_symtab != nullptr,
            "The Subroutine::m_symtab cannot be nullptr");
        require(x.m_symtab->parent == parent_symtab,
            "The Subroutine::m_symtab->parent is not the right parent");
        for (auto &a : x.m_symtab->scope) {
            symbol_enter_ok = true;
            this->visit_symbol(*a.second);
            symbol_enter_ok = false;
        }
        symbol_enter_ok = false;
        for (size_t i=0; i<x.n_args; i++) {
            visit_expr(*x.m_args[i]);
        }
        for (size_t i=0; i<x.n_body; i++) {
            visit_stmt(*x.m_body[i]);
        }
        current_symtab = parent_symtab;
    }

    void visit_Function(const Function_t &x) {
        if (!symbol_enter_ok) return;
        SymbolTable *parent_symtab = current_symtab;
        current_symtab = x.m_symtab;
        require(x.m_symtab != nullptr,
            "The Function::m_symtab cannot be nullptr");
        require(x.m_symtab->parent == parent_symtab,
            "The Function::m_symtab->parent is not the right parent");
        for (auto &a : x.m_symtab->scope) {
            symbol_enter_ok = true;
            this->visit_symbol(*a.second);
            symbol_enter_ok = false;
        }
        for (size_t i=0; i<x.n_args; i++) {
            visit_expr(*x.m_args[i]);
        }
        for (size_t i=0; i<x.n_body; i++) {
            visit_stmt(*x.m_body[i]);
        }
        visit_expr(*x.m_return_var);
        current_symtab = parent_symtab;
    }

    void visit_ExternalSymbol(const ExternalSymbol_t &x) {
        symbol_enter_ok = false;
        visit_symbol(*x.m_external);
    }

    void visit_Var(const Var_t &x) {
        require(x.m_v != nullptr,
            "Var_t::m_v cannot be nullptr");
        require(is_a<Variable_t>(*x.m_v),
            "Var_t::m_v does not point to a Variable_t");
        visit_symbol(*x.m_v);
    }

    void visit_Variable(const Variable_t &x) {
        if (!symbol_enter_ok) return;
        SymbolTable *symtab = x.m_parent_symtab;
        require(symtab != nullptr,
            "Variable::m_parent_symtab cannot be nullptr");
        require(symtab->scope.find(std::string(x.m_name)) != symtab->scope.end(),
            "Variable not found in parent_symtab symbol table");
        symbol_t *symtab_sym = symtab->scope[std::string(x.m_name)];
        const symbol_t *current_sym = &x.base;
        require(symtab_sym == current_sym,
            "Variable's parent symbol table does not point to it");

        if (x.m_value)
            visit_expr(*x.m_value);
        visit_ttype(*x.m_type);
    }

};


} // namespace ASR

bool asr_verify(const ASR::TranslationUnit_t &unit) {
    ASR::VerifyVisitor v;
    v.visit_TranslationUnit(unit);
    return true;
}

} // namespace LFortran
