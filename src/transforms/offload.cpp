#include <taichi/taichi>
#include <set>
#include "../ir.h"

TLANG_NAMESPACE_BEGIN

namespace irpass {

class Offloader {
 public:
  Offloader(IRNode *root) {
    run(root);
  }

  void run(IRNode *root) {
    auto root_block = dynamic_cast<Block *>(root);
    auto &root_statements = root_block->statements;
    auto new_root_statements = std::vector<pStmt>();

    for (int i = 0; i < (int)root_statements.size(); i++) {
      auto &stmt = root_statements[i];
      if (auto s = stmt->cast<RangeForStmt>()) {
        auto offloaded =
            Stmt::make_typed<OffloadedStmt>(OffloadedStmt::TaskType::range_for);
        offloaded->body_stmt = std::move(root_statements[i]);
        new_root_statements.push_back(std::move(offloaded));
      } else if (auto s = stmt->cast<StructForStmt>()) {
        // TODO: emit listgen
        auto offloaded =
            Stmt::make_typed<OffloadedStmt>(OffloadedStmt::TaskType::struct_for);
        offloaded->body_stmt = std::move(root_statements[i]);
        new_root_statements.push_back(std::move(offloaded));
      } else {
        // Serial stmt
        auto offloaded =
            Stmt::make_typed<OffloadedStmt>(OffloadedStmt::TaskType::serial);
        offloaded->body_stmt = std::move(root_statements[i]);
        new_root_statements.push_back(std::move(offloaded));
      }
    }

    root_block->statements = std::move(new_root_statements);
  }
};

void offload(IRNode *root) {
  Offloader _(root);
}

}  // namespace irpass

TLANG_NAMESPACE_END
