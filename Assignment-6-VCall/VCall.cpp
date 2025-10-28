// /**
//  * Andersen.cpp
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
    // consg->dump();
    
    Andersen andersen(consg, pag);  // 传入 pag
    auto cg = pag->getCallGraph();
    
    // 完成以下两个方法
    andersen.runPointerAnalysis();
    andersen.updateCallGraph(cg);
    
    // cg->dump();
    SVF::LLVMModuleSet::releaseLLVMModuleSet();
    return 0;
}

void Andersen::runPointerAnalysis()
{
    // 工作列表：存储需要处理的节点
    WorkList<SVF::NodeID> worklist;
    
    // Step 1: 初始化 - 处理所有的 Address 约束 (p = &a)
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

void Andersen::updateCallGraph(SVF::CallGraph* cg)
{
    // 使用 PAG 来实现，因为 ConstraintGraph 没有 isFunction/getFunction
    
    // 获取所有间接调用点
    const auto& indirectCallsites = consg->getIndirectCallsites();
    
    // 遍历每个间接调用点
    for (auto& pair : indirectCallsites) {
        const SVF::CallICFGNode* callNode = pair.first;
        SVF::NodeID funPtrId = pair.second;
        
        // 获取调用者
        const SVF::FunObjVar* callerFun = callNode->getCaller();
        
        // 获取函数指针的 points-to 集合
        const std::set<unsigned>& ptsSet = pts[funPtrId];
        
        // 遍历 points-to 集合
        for (SVF::NodeID objId : ptsSet) {
            // 从 PAG 获取节点
            SVF::PAGNode* pagNode = pag->getGNode(objId);
            
            // 检查是否是函数对象变量 (FunObjVar)
            if (SVF::FunObjVar* funObjVar = SVF::SVFUtil::dyn_cast<SVF::FunObjVar>(pagNode)) {
                // 添加间接调用边
                cg->addIndirectCallGraphEdge(callNode, callerFun, funObjVar);
            }
        }
    }
}
