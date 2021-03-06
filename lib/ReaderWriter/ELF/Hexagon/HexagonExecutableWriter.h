//===- lib/ReaderWriter/ELF/Hexagon/HexagonExecutableWriter.h -------------===//
//
//                             The LLVM Linker
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
#ifndef HEXAGON_EXECUTABLE_WRITER_H
#define HEXAGON_EXECUTABLE_WRITER_H

#include "ExecutableWriter.h"
#include "HexagonExecutableAtoms.h"
#include "HexagonLinkingContext.h"

namespace lld {
namespace elf {

template <typename ELFT> class HexagonTargetLayout;

template <class ELFT>
class HexagonExecutableWriter : public ExecutableWriter<ELFT> {
public:
  HexagonExecutableWriter(HexagonLinkingContext &ctx,
                          HexagonTargetLayout<ELFT> &layout);

protected:
  // Add any runtime files and their atoms to the output
  bool createImplicitFiles(std::vector<std::unique_ptr<File>> &) override;

  void finalizeDefaultAtomValues() override;

  std::error_code setELFHeader() override {
    ExecutableWriter<ELFT>::setELFHeader();
    setHexagonELFHeader(*this->_elfHeader);
    return std::error_code();
  }

private:
  void addDefaultAtoms() override {
    _runtimeFile->addAbsoluteAtom("_SDA_BASE_");
    if (this->_ctx.isDynamic()) {
      _runtimeFile->addAbsoluteAtom("_GLOBAL_OFFSET_TABLE_");
      _runtimeFile->addAbsoluteAtom("_DYNAMIC");
    }
  }

  HexagonLinkingContext &_ctx;
  HexagonTargetLayout<ELFT> &_targetLayout;
  std::unique_ptr<HexagonRuntimeFile<ELFT>> _runtimeFile;
};

template <class ELFT>
HexagonExecutableWriter<ELFT>::HexagonExecutableWriter(
    HexagonLinkingContext &ctx, HexagonTargetLayout<ELFT> &layout)
    : ExecutableWriter<ELFT>(ctx, layout), _ctx(ctx), _targetLayout(layout),
      _runtimeFile(new HexagonRuntimeFile<ELFT>(ctx)) {}

template <class ELFT>
bool HexagonExecutableWriter<ELFT>::createImplicitFiles(
    std::vector<std::unique_ptr<File>> &result) {
  ExecutableWriter<ELFT>::createImplicitFiles(result);
  // Add the default atoms as defined for hexagon
  addDefaultAtoms();
  result.push_back(std::move(_runtimeFile));
  return true;
}

template <class ELFT>
void HexagonExecutableWriter<ELFT>::finalizeDefaultAtomValues() {
  // Finalize the atom values that are part of the parent.
  ExecutableWriter<ELFT>::finalizeDefaultAtomValues();
  AtomLayout *sdabaseAtom = _targetLayout.findAbsoluteAtom("_SDA_BASE_");
  sdabaseAtom->_virtualAddr = _targetLayout.getSDataSection()->virtualAddr();
  if (_ctx.isDynamic())
    finalizeHexagonRuntimeAtomValues(_targetLayout);
}

} // namespace elf
} // namespace lld

#endif // HEXAGON_EXECUTABLE_WRITER_H
