// (c) Copyright 2016-2022 Xilinx, Inc.
// All Rights Reserved.
//
// Licensed to the Apache Software Foundation (ASF) under one
// or more contributor license agreements.  See the NOTICE file
// distributed with this work for additional information
// regarding copyright ownership.  The ASF licenses this file
// to you under the Apache License, Version 2.0 (the
// "License"); you may not use this file except in compliance
// with the License.  You may obtain a copy of the License at
//
//   http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing,
// software distributed under the License is distributed on an
// "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
// KIND, either express or implied.  See the License for the
// specific language governing permissions and limitations
// under the License.

#ifndef LLVM_CLANG_TOOLS_EXTRA_CLANG_TIDY_XILINX_COMMON_H
#define LLVM_CLANG_TOOLS_EXTRA_CLANG_TIDY_XILINX_COMMON_H

#include "../ClangTidy.h"

namespace clang {
namespace tidy {
namespace xilinx {

inline std::string dumpLineInfo(SourceLocation Loc, SourceManager *SM) {
  // keep line number as source code
  PresumedLoc PLoc = SM->getPresumedLoc(Loc);
  if (Loc.isInvalid() || !Loc.isFileID() || PLoc.isInvalid())
    return "";

  std::string Filename = PLoc.getFilename();
  llvm::SmallString<256> x; 
  llvm::raw_svector_ostream  os( x ); 
  os.write_escaped(Filename); 
  Filename = os.str(); 

  std::string LineInfo =
      "# " + std::to_string(PLoc.getLine()) + " \"" + Filename + "\"" + "\n";
  return LineInfo;
}

} // namespace xilinx
} // namespace tidy
} // namespace clang

#endif // LLVM_CLANG_TOOLS_EXTRA_CLANG_TIDY_XILINX_COMMON_H
