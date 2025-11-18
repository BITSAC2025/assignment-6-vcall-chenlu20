// // /**
// //  * Andersen.cpp
// //  * @author kisslune
// //  */

// // #include "A6Header.h"

// // using namespace llvm;
// // using namespace std;

// // int main(int argc, char** argv)
// // {
// //     auto moduleNameVec =
// //             OptionBase::parseOptions(argc, argv, "Whole Program Points-to Analysis",
// //                                      "[options] <input-bitcode...>");

// //     SVF::LLVMModuleSet::buildSVFModule(moduleNameVec);

// //     SVF::SVFIRBuilder builder;
// //     auto pag = builder.build();
// //     auto consg = new SVF::ConstraintGraph(pag);
// //     consg->dump();

// //     Andersen andersen(consg);
// //     auto cg = pag->getCallGraph();

// //     // TODO: complete the following two methods
// //     andersen.runPointerAnalysis();
// //     andersen.updateCallGraph(cg);

// //     cg->dump();
// //     SVF::LLVMModuleSet::releaseLLVMModuleSet();
// // 	return 0;
// // }


// // void Andersen::runPointerAnalysis()
// // {
// //     // TODO: complete this method. Point-to set and worklist are defined in A5Header.h
// //     //  The implementation of constraint graph is provided in the SVF library
// // }


// // void Andersen::updateCallGraph(SVF::CallGraph* cg)
// // {
// //     // TODO: complete this method.
// //     //  The implementation of call graph is provided in the SVF library
// // }


// /**
//  * VCall.cpp
//  * @author kisslune
//  * 
//  * Assignment 6 - VCall Analysis Implementation
//  * Student Name: 胡晨璐
//  * Student ID: 3220252746
//  */
// #include "A6Header.h"
// using namespace llvm;
// using namespace std;

// int main(int argc, char** argv)
// {
//     auto moduleNameVec =
//             OptionBase::parseOptions(argc, argv, "Whole Program Points-to Analysis",
//                                      "[options] <input-bitcode...>");
//     SVF::LLVMModuleSet::buildSVFModule(moduleNameVec);
//     SVF::SVFIRBuilder builder;
//     auto pag = builder.build();
//     auto consg = new SVF::ConstraintGraph(pag);
    
//     Andersen andersen(consg, pag);
//     auto cg = pag->getCallGraph();
    
//     andersen.runPointerAnalysis();
//     andersen.updateCallGraph(cg);
    
//     SVF::LLVMModuleSet::releaseLLVMModuleSet();
//     return 0;
// }

// void Andersen::runPointerAnalysis()
// {
//     WorkList<SVF::NodeID> worklist;
    
//     // Step 1: 初始化 - 处理所有的 Address 约束
//     for (auto iter = consg->begin(); iter != consg->end(); ++iter) {
//         SVF::ConstraintNode* node = iter->second;
//         const SVF::ConstraintEdge::ConstraintEdgeSetTy& addrInEdges = node->getAddrInEdges();
        
//         for (auto edge : addrInEdges) {
//             SVF::NodeID srcId = edge->getSrcID();
//             SVF::NodeID dstId = edge->getDstID();
            
//             if (pts[dstId].insert(srcId).second) {
//                 worklist.push(dstId);
//             }
//         }
//     }
    
//     // Step 2: 主循环
//     while (!worklist.empty()) {
//         SVF::NodeID nodeId = worklist.pop();
//         SVF::ConstraintNode* node = consg->getConstraintNode(nodeId);
//         const std::set<unsigned>& nodePts = pts[nodeId];
        
//         // Copy 边
//         const SVF::ConstraintEdge::ConstraintEdgeSetTy& copyOutEdges = node->getCopyOutEdges();
//         for (auto edge : copyOutEdges) {
//             SVF::NodeID dstId = edge->getDstID();
//             size_t oldSize = pts[dstId].size();
//             pts[dstId].insert(nodePts.begin(), nodePts.end());
//             if (pts[dstId].size() > oldSize) {
//                 worklist.push(dstId);
//             }
//         }
        
//         // Load 边
//         const SVF::ConstraintEdge::ConstraintEdgeSetTy& loadOutEdges = node->getLoadOutEdges();
//         for (auto edge : loadOutEdges) {
//             SVF::LoadCGEdge* loadEdge = SVF::SVFUtil::dyn_cast<SVF::LoadCGEdge>(edge);
//             if (!loadEdge) continue;
            
//             SVF::NodeID dstId = loadEdge->getDstID();
//             size_t oldSize = pts[dstId].size();
            
//             for (SVF::NodeID o : nodePts) {
//                 pts[dstId].insert(pts[o].begin(), pts[o].end());
//             }
            
//             if (pts[dstId].size() > oldSize) {
//                 worklist.push(dstId);
//             }
//         }
        
//         // Store 边
//         const SVF::ConstraintEdge::ConstraintEdgeSetTy& storeOutEdges = node->getStoreOutEdges();
//         for (auto edge : storeOutEdges) {
//             SVF::StoreCGEdge* storeEdge = SVF::SVFUtil::dyn_cast<SVF::StoreCGEdge>(edge);
//             if (!storeEdge) continue;
            
//             SVF::NodeID srcId = storeEdge->getSrcID();
//             const std::set<unsigned>& srcPts = pts[srcId];
            
//             for (SVF::NodeID o : nodePts) {
//                 size_t oldSize = pts[o].size();
//                 pts[o].insert(srcPts.begin(), srcPts.end());
//                 if (pts[o].size() > oldSize) {
//                     worklist.push(o);
//                 }
//             }
//         }
        
//         // Gep 边
//         const SVF::ConstraintEdge::ConstraintEdgeSetTy& gepOutEdges = node->getGepOutEdges();
//         for (auto edge : gepOutEdges) {
//             SVF::GepCGEdge* gepEdge = SVF::SVFUtil::dyn_cast<SVF::GepCGEdge>(edge);
//             if (!gepEdge) continue;
            
//             SVF::NodeID dstId = gepEdge->getDstID();
//             bool changed = false;
//             // GEP边通常offset为0，或者可以尝试从边获取
//             SVF::APOffset offset = 0;
            
//             for (SVF::NodeID o : nodePts) {
//                 SVF::NodeID fieldObj = consg->getGepObjVar(o, offset);
//                 if (pts[dstId].insert(fieldObj).second) {
//                     changed = true;
//                 }
//             }
            
//             if (changed) {
//                 worklist.push(dstId);
//             }
//         }
//     }
// }

// void Andersen::updateCallGraph(SVF::CallGraph* cg)
// {
//     const auto& indirectCallsites = consg->getIndirectCallsites();
    
//     for (auto& pair : indirectCallsites) {
//         const SVF::CallICFGNode* callNode = pair.first;
//         SVF::NodeID funPtrId = pair.second;
        
//         // getCaller() 返回 FunObjVar*
//         const SVF::FunObjVar* caller = callNode->getCaller();
        
//         const std::set<unsigned>& ptsSet = pts[funPtrId];
        
//         for (SVF::NodeID objId : ptsSet) {
//             SVF::PAGNode* pagNode = pag->getGNode(objId);
            
//             // 检查是否是函数对象
//             if (SVF::FunObjVar* callee = SVF::SVFUtil::dyn_cast<SVF::FunObjVar>(pagNode)) {
//                 // addIndirectCallGraphEdge 需要两个 FunObjVar*
//                 cg->addIndirectCallGraphEdge(callNode, caller, callee);
//             }
//         }
//     }
// }

// /**
//  * Vcall.cpp
//  * @author kisslune
//  */

// #include "A6Header.h"

// using namespace llvm;
// using namespace std;

// int main(int argc, char** argv)
// {
//     auto moduleNameVec =
//             OptionBase::parseOptions(argc, argv, "Whole Program Points-to Analysis",
//                                      "[options] <input-bitcode...>");

//     SVF::LLVMModuleSet::buildSVFModule(moduleNameVec);

//     SVF::SVFIRBuilder builder;
//     auto pag = builder.build();
//     auto consg = new SVF::ConstraintGraph(pag);
//     consg->dump();

//     Andersen andersen(consg);
//     auto cg = pag->getCallGraph();

//     // TODO: complete the following two methods
//     andersen.runPointerAnalysis();
//     andersen.updateCallGraph(cg);

//     cg->dump();
//     SVF::LLVMModuleSet::releaseLLVMModuleSet();
// 	return 0;
// }


// void Andersen::runPointerAnalysis()
// {
//     // TODO: complete this method. Point-to set and worklist are defined in A5Header.h
//     //  The implementation of constraint graph is provided in the SVF library
// }


// void Andersen::updateCallGraph(SVF::CallGraph* cg)
// {
//     // TODO: complete this method.
//     //  The implementation of call graph is provided in the SVF library
// }


/**
 * VCall.cpp
 * @author kisslune
 * 
 * Assignment 6 - VCall Analysis Implementation
 * Student Name: 胡晨璐
 * Student ID: 3220252746
 */
#include "A6Header.h"
using namespace llvm;
using namespace std;

int main(int argc, char** argv)
{
    auto moduleNameVec =
            OptionBase::parseOptions(argc, argv, "Whole Program Points-to Analysis",
                                     "[options] <input-bitcode...>");
    SVF::LLVMModuleSet::buildSVFModule(moduleNameVec);
    SVF::SVFIRBuilder builder;
    auto pag = builder.build();
    auto consg = new SVF::ConstraintGraph(pag);
    consg->dump();  // ← 添加这行！
    
    Andersen andersen(consg, pag);
    auto cg = pag->getCallGraph();
    
    andersen.runPointerAnalysis();
    andersen.updateCallGraph(cg);

    cg->dump();  // ← 添加这行！
    SVF::LLVMModuleSet::releaseLLVMModuleSet();
    return 0;
}

void Andersen::runPointerAnalysis()
{
    WorkList<SVF::NodeID> worklist;
    
    // Step 1: 初始化 - 处理所有的 Address 约束
    for (auto iter = consg->begin(); iter != consg->end(); ++iter) {
        SVF::ConstraintNode* node = iter->second;
        const SVF::ConstraintEdge::ConstraintEdgeSetTy& addrInEdges = node->getAddrInEdges();
        
        for (auto edge : addrInEdges) {
            SVF::NodeID srcId = edge->getSrcID();
            SVF::NodeID dstId = edge->getDstID();
            
            if (pts[dstId].insert(srcId).second) {
                worklist.push(dstId);
            }
        }
    }
    
    // Step 2: 主循环
    while (!worklist.empty()) {
        SVF::NodeID nodeId = worklist.pop();
        SVF::ConstraintNode* node = consg->getConstraintNode(nodeId);
        const std::set<unsigned>& nodePts = pts[nodeId];
        
        // Copy 边
        const SVF::ConstraintEdge::ConstraintEdgeSetTy& copyOutEdges = node->getCopyOutEdges();
        for (auto edge : copyOutEdges) {
            SVF::NodeID dstId = edge->getDstID();
            size_t oldSize = pts[dstId].size();
            pts[dstId].insert(nodePts.begin(), nodePts.end());
            if (pts[dstId].size() > oldSize) {
                worklist.push(dstId);
            }
        }
        
        // Load 边
        const SVF::ConstraintEdge::ConstraintEdgeSetTy& loadOutEdges = node->getLoadOutEdges();
        for (auto edge : loadOutEdges) {
            SVF::LoadCGEdge* loadEdge = SVF::SVFUtil::dyn_cast<SVF::LoadCGEdge>(edge);
            if (!loadEdge) continue;
            
            SVF::NodeID dstId = loadEdge->getDstID();
            size_t oldSize = pts[dstId].size();
            
            for (SVF::NodeID o : nodePts) {
                pts[dstId].insert(pts[o].begin(), pts[o].end());
            }
            
            if (pts[dstId].size() > oldSize) {
                worklist.push(dstId);
            }
        }
        
        // Store 边
        const SVF::ConstraintEdge::ConstraintEdgeSetTy& storeOutEdges = node->getStoreOutEdges();
        for (auto edge : storeOutEdges) {
            SVF::StoreCGEdge* storeEdge = SVF::SVFUtil::dyn_cast<SVF::StoreCGEdge>(edge);
            if (!storeEdge) continue;
            
            SVF::NodeID srcId = storeEdge->getSrcID();
            const std::set<unsigned>& srcPts = pts[srcId];
            
            for (SVF::NodeID o : nodePts) {
                size_t oldSize = pts[o].size();
                pts[o].insert(srcPts.begin(), srcPts.end());
                if (pts[o].size() > oldSize) {
                    worklist.push(o);
                }
            }
        }
        
        // Gep 边
        const SVF::ConstraintEdge::ConstraintEdgeSetTy& gepOutEdges = node->getGepOutEdges();
        for (auto edge : gepOutEdges) {
            SVF::GepCGEdge* gepEdge = SVF::SVFUtil::dyn_cast<SVF::GepCGEdge>(edge);
            if (!gepEdge) continue;
            
            SVF::NodeID dstId = gepEdge->getDstID();
            bool changed = false;
            // GEP边通常offset为0，或者可以尝试从边获取
            SVF::APOffset offset = 0;
            
            for (SVF::NodeID o : nodePts) {
                SVF::NodeID fieldObj = consg->getGepObjVar(o, offset);
                if (pts[dstId].insert(fieldObj).second) {
                    changed = true;
                }
            }
            
            if (changed) {
                worklist.push(dstId);
            }
        }
    }
}

// void Andersen::updateCallGraph(SVF::CallGraph* cg)
// {
//     const auto& indirectCallsites = consg->getIndirectCallsites();
    
//     for (auto& pair : indirectCallsites) {
//         const SVF::CallICFGNode* callNode = pair.first;
//         SVF::NodeID funPtrId = pair.second;
        
//         // getCaller() 返回 FunObjVar*
//         const SVF::FunObjVar* caller = callNode->getCaller();
        
//         const std::set<unsigned>& ptsSet = pts[funPtrId];
        
//         for (SVF::NodeID objId : ptsSet) {
//             SVF::PAGNode* pagNode = pag->getGNode(objId);
            
//             // 检查是否是函数对象
//             if (SVF::FunObjVar* callee = SVF::SVFUtil::dyn_cast<SVF::FunObjVar>(pagNode)) {
//                 // addIndirectCallGraphEdge 需要两个 FunObjVar*
//                 cg->addIndirectCallGraphEdge(callNode, caller, callee);
//             }
//         }
//     }
// }
void Andersen::updateCallGraph(SVF::CallGraph* cg)
{
    const auto& indirectCallsites = consg->getIndirectCallsites();
    
    for (auto& pair : indirectCallsites) {
        const SVF::CallICFGNode* callNode = pair.first;
        SVF::NodeID funPtrId = pair.second;
        
        // ✅ 正确：getCaller() 直接返回 FunObjVar*
        const SVF::FunObjVar* caller = callNode->getCaller();
        
        const std::set<unsigned>& ptsSet = pts[funPtrId];
        
        for (SVF::NodeID objId : ptsSet) {
            SVF::PAGNode* pagNode = pag->getGNode(objId);
            
            // ✅ 直接检查是否为FunObjVar类型，不需要调用getFunction()
            if (SVF::FunObjVar* callee = SVF::SVFUtil::dyn_cast<SVF::FunObjVar>(pagNode)) {
                // ✅ 两个参数都是FunObjVar*类型
                cg->addIndirectCallGraphEdge(callNode, caller, callee);
            }
        }
    }
}
