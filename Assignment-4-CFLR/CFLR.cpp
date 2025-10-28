// /**
//  * CFLR.cpp
//  * @author kisslune 
//  */

// #include "A4Header.h"

// using namespace SVF;
// using namespace llvm;
// using namespace std;

// int main(int argc, char **argv)
// {
//     auto moduleNameVec =
//             OptionBase::parseOptions(argc, argv, "Whole Program Points-to Analysis",
//                                      "[options] <input-bitcode...>");

//     LLVMModuleSet::buildSVFModule(moduleNameVec);

//     SVFIRBuilder builder;
//     auto pag = builder.build();
//     pag->dump();

//     CFLR solver;
//     solver.buildGraph(pag);
//     // TODO: complete this method
//     solver.solve();
//     solver.dumpResult();

//     LLVMModuleSet::releaseLLVMModuleSet();
//     return 0;
// }


// void CFLR::solve()
// {
//     // TODO: complete this function. The implementations of graph and worklist are provided.
//     //  You need to:
//     //  1. implement the grammar production rules into code;
//     //  2. implement the dynamic-programming CFL-reachability algorithm.
//     //  You may need to add your new methods to 'CFLRGraph' and 'CFLR'.
// }


/**
 * CFLR.cpp
 * @author kisslune 
 */

#include "A4Header.h"

using namespace SVF;
using namespace llvm;
using namespace std;

int main(int argc, char **argv)
{
    auto moduleNameVec =
            OptionBase::parseOptions(argc, argv, "Whole Program Points-to Analysis",
                                     "[options] <input-bitcode...>");

    LLVMModuleSet::buildSVFModule(moduleNameVec);

    SVFIRBuilder builder;
    auto pag = builder.build();
    pag->dump();

    CFLR solver;
    solver.buildGraph(pag);
    // TODO: complete this method
    solver.solve();
    solver.dumpResult();

    LLVMModuleSet::releaseLLVMModuleSet();
    return 0;
}


// void CFLR::solve()
// {
//     // TODO: complete this function. The implementations of graph and worklist are provided.
//     //  You need to:
//     //  1. implement the grammar production rules into code;
//     //  2. implement the dynamic-programming CFL-reachability algorithm.
//     //  You may need to add your new methods to 'CFLRGraph' and 'CFLR'.
// }

void CFLR::solve()
{
    // Step 1: 初始化worklist
    std::cout << "Initializing worklist with existing edges..." << std::endl;
    
    for (auto &[src, labelMap] : graph->getSuccessorMap())
    {
        for (auto &[label, dstSet] : labelMap)
        {
            for (unsigned dst : dstSet)
            {
                workList.push(CFLREdge(src, dst, label));
            }
        }
    }
    
    // Step 2: 添加 epsilon 边 (VF ::= ε)
    std::cout << "Adding epsilon edges (VF self-loops)..." << std::endl;
    
    std::unordered_set<unsigned> nodes;
    for (auto &[src, labelMap] : graph->getSuccessorMap())
    {
        nodes.insert(src);
        for (auto &[label, dstSet] : labelMap)
        {
            for (unsigned dst : dstSet)
            {
                nodes.insert(dst);
            }
        }
    }
    
    for (unsigned node : nodes)
    {
        if (!graph->hasEdge(node, node, EdgeLabelType::VF))
        {
            graph->addEdge(node, node, EdgeLabelType::VF);
            workList.push(CFLREdge(node, node, EdgeLabelType::VF));
        }
    }
    
    // Step 3: 主循环
    std::cout << "Starting CFL-Reachability main loop..." << std::endl;
    int iteration = 0;
    
    while (!workList.empty())
    {
        iteration++;
        if (iteration % 10000 == 0)
        {
            std::cout << "Processed " << iteration << " edges" << std::endl;
        }
        
        CFLREdge edge = workList.pop();
        unsigned vi = edge.src;
        unsigned vj = edge.dst;
        EdgeLabel X = edge.label;
        
        // ============================================
        // 单符号产生式: VF ::= Copy
        // ============================================
        if (X == EdgeLabelType::Copy)
        {
            if (!graph->hasEdge(vi, vj, EdgeLabelType::VF))
            {
                graph->addEdge(vi, vj, EdgeLabelType::VF);
                workList.push(CFLREdge(vi, vj, EdgeLabelType::VF));
            }
        }
        
        // ============================================
        // 向前查找: vi →^X vj, 查找 vj →^Y vk
        // ============================================
        auto &succMap = graph->getSuccessorMap();  // ✅ 每次重新获取
        if (succMap.find(vj) != succMap.end())
        {
            auto &vjSucc = succMap.at(vj);  // ✅ 使用 at() 更安全
            
            // PT ::= VF Addr
            if (X == EdgeLabelType::VF && vjSucc.find(EdgeLabelType::Addr) != vjSucc.end())
            {
                for (unsigned vk : vjSucc.at(EdgeLabelType::Addr))
                {
                    if (!graph->hasEdge(vi, vk, EdgeLabelType::PT))
                    {
                        graph->addEdge(vi, vk, EdgeLabelType::PT);
                        workList.push(CFLREdge(vi, vk, EdgeLabelType::PT));
                    }
                }
            }
            
            // PT ::= Addr VF
            if (X == EdgeLabelType::Addr && vjSucc.find(EdgeLabelType::VF) != vjSucc.end())
            {
                for (unsigned vk : vjSucc.at(EdgeLabelType::VF))
                {
                    if (!graph->hasEdge(vi, vk, EdgeLabelType::PT))
                    {
                        graph->addEdge(vi, vk, EdgeLabelType::PT);
                        workList.push(CFLREdge(vi, vk, EdgeLabelType::PT));
                    }
                }
            }
            
            // VF ::= VF VF
            if (X == EdgeLabelType::VF && vjSucc.find(EdgeLabelType::VF) != vjSucc.end())
            {
                for (unsigned vk : vjSucc.at(EdgeLabelType::VF))
                {
                    if (!graph->hasEdge(vi, vk, EdgeLabelType::VF))
                    {
                        graph->addEdge(vi, vk, EdgeLabelType::VF);
                        workList.push(CFLREdge(vi, vk, EdgeLabelType::VF));
                    }
                }
            }
            
            // VF ::= SV Load
            if (X == EdgeLabelType::SV && vjSucc.find(EdgeLabelType::Load) != vjSucc.end())
            {
                for (unsigned vk : vjSucc.at(EdgeLabelType::Load))
                {
                    if (!graph->hasEdge(vi, vk, EdgeLabelType::VF))
                    {
                        graph->addEdge(vi, vk, EdgeLabelType::VF);
                        workList.push(CFLREdge(vi, vk, EdgeLabelType::VF));
                    }
                }
            }
            
            // VF ::= PV Load
            if (X == EdgeLabelType::PV && vjSucc.find(EdgeLabelType::Load) != vjSucc.end())
            {
                for (unsigned vk : vjSucc.at(EdgeLabelType::Load))
                {
                    if (!graph->hasEdge(vi, vk, EdgeLabelType::VF))
                    {
                        graph->addEdge(vi, vk, EdgeLabelType::VF);
                        workList.push(CFLREdge(vi, vk, EdgeLabelType::VF));
                    }
                }
            }
            
            // VF ::= Store VP
            if (X == EdgeLabelType::Store && vjSucc.find(EdgeLabelType::VP) != vjSucc.end())
            {
                for (unsigned vk : vjSucc.at(EdgeLabelType::VP))
                {
                    if (!graph->hasEdge(vi, vk, EdgeLabelType::VF))
                    {
                        graph->addEdge(vi, vk, EdgeLabelType::VF);
                        workList.push(CFLREdge(vi, vk, EdgeLabelType::VF));
                    }
                }
            }
            
            // SV ::= Store VA
            if (X == EdgeLabelType::Store && vjSucc.find(EdgeLabelType::VA) != vjSucc.end())
            {
                for (unsigned vk : vjSucc.at(EdgeLabelType::VA))
                {
                    if (!graph->hasEdge(vi, vk, EdgeLabelType::SV))
                    {
                        graph->addEdge(vi, vk, EdgeLabelType::SV);
                        workList.push(CFLREdge(vi, vk, EdgeLabelType::SV));
                    }
                }
            }
            
            // SV ::= VA Store
            if (X == EdgeLabelType::VA && vjSucc.find(EdgeLabelType::Store) != vjSucc.end())
            {
                for (unsigned vk : vjSucc.at(EdgeLabelType::Store))
                {
                    if (!graph->hasEdge(vi, vk, EdgeLabelType::SV))
                    {
                        graph->addEdge(vi, vk, EdgeLabelType::SV);
                        workList.push(CFLREdge(vi, vk, EdgeLabelType::SV));
                    }
                }
            }
            
            // PV ::= PT VA
            if (X == EdgeLabelType::PT && vjSucc.find(EdgeLabelType::VA) != vjSucc.end())
            {
                for (unsigned vk : vjSucc.at(EdgeLabelType::VA))
                {
                    if (!graph->hasEdge(vi, vk, EdgeLabelType::PV))
                    {
                        graph->addEdge(vi, vk, EdgeLabelType::PV);
                        workList.push(CFLREdge(vi, vk, EdgeLabelType::PV));
                    }
                }
            }
            
            // VP ::= VA PT
            if (X == EdgeLabelType::VA && vjSucc.find(EdgeLabelType::PT) != vjSucc.end())
            {
                for (unsigned vk : vjSucc.at(EdgeLabelType::PT))
                {
                    if (!graph->hasEdge(vi, vk, EdgeLabelType::VP))
                    {
                        graph->addEdge(vi, vk, EdgeLabelType::VP);
                        workList.push(CFLREdge(vi, vk, EdgeLabelType::VP));
                    }
                }
            }
            
            // LV ::= Load VA
            if (X == EdgeLabelType::Load && vjSucc.find(EdgeLabelType::VA) != vjSucc.end())
            {
                for (unsigned vk : vjSucc.at(EdgeLabelType::VA))
                {
                    if (!graph->hasEdge(vi, vk, EdgeLabelType::LV))
                    {
                        graph->addEdge(vi, vk, EdgeLabelType::LV);
                        workList.push(CFLREdge(vi, vk, EdgeLabelType::LV));
                    }
                }
            }
        }
        
        // ============================================
        // 向后查找: vi →^X vj, 查找 vk →^Y vi
        // ============================================
        auto &predMap = graph->getPredecessorMap();  // ✅ 每次重新获取
        if (predMap.find(vi) != predMap.end())
        {
            auto &viPred = predMap.at(vi);  // ✅ 使用 at() 更安全
            
            // PT ::= VF Addr
            if (X == EdgeLabelType::Addr && viPred.find(EdgeLabelType::VF) != viPred.end())
            {
                for (unsigned vk : viPred.at(EdgeLabelType::VF))
                {
                    if (!graph->hasEdge(vk, vj, EdgeLabelType::PT))
                    {
                        graph->addEdge(vk, vj, EdgeLabelType::PT);
                        workList.push(CFLREdge(vk, vj, EdgeLabelType::PT));
                    }
                }
            }
            
            // PT ::= Addr VF
            if (X == EdgeLabelType::VF && viPred.find(EdgeLabelType::Addr) != viPred.end())
            {
                for (unsigned vk : viPred.at(EdgeLabelType::Addr))
                {
                    if (!graph->hasEdge(vk, vj, EdgeLabelType::PT))
                    {
                        graph->addEdge(vk, vj, EdgeLabelType::PT);
                        workList.push(CFLREdge(vk, vj, EdgeLabelType::PT));
                    }
                }
            }
            
            // VF ::= VF VF
            if (X == EdgeLabelType::VF && viPred.find(EdgeLabelType::VF) != viPred.end())
            {
                for (unsigned vk : viPred.at(EdgeLabelType::VF))
                {
                    if (!graph->hasEdge(vk, vj, EdgeLabelType::VF))
                    {
                        graph->addEdge(vk, vj, EdgeLabelType::VF);
                        workList.push(CFLREdge(vk, vj, EdgeLabelType::VF));
                    }
                }
            }
            
            // VF ::= SV Load
            if (X == EdgeLabelType::Load && viPred.find(EdgeLabelType::SV) != viPred.end())
            {
                for (unsigned vk : viPred.at(EdgeLabelType::SV))
                {
                    if (!graph->hasEdge(vk, vj, EdgeLabelType::VF))
                    {
                        graph->addEdge(vk, vj, EdgeLabelType::VF);
                        workList.push(CFLREdge(vk, vj, EdgeLabelType::VF));
                    }
                }
            }
            
            // VF ::= PV Load
            if (X == EdgeLabelType::Load && viPred.find(EdgeLabelType::PV) != viPred.end())
            {
                for (unsigned vk : viPred.at(EdgeLabelType::PV))
                {
                    if (!graph->hasEdge(vk, vj, EdgeLabelType::VF))
                    {
                        graph->addEdge(vk, vj, EdgeLabelType::VF);
                        workList.push(CFLREdge(vk, vj, EdgeLabelType::VF));
                    }
                }
            }
            
            // VF ::= Store VP
            if (X == EdgeLabelType::VP && viPred.find(EdgeLabelType::Store) != viPred.end())
            {
                for (unsigned vk : viPred.at(EdgeLabelType::Store))
                {
                    if (!graph->hasEdge(vk, vj, EdgeLabelType::VF))
                    {
                        graph->addEdge(vk, vj, EdgeLabelType::VF);
                        workList.push(CFLREdge(vk, vj, EdgeLabelType::VF));
                    }
                }
            }
            
            // SV ::= Store VA
            if (X == EdgeLabelType::VA && viPred.find(EdgeLabelType::Store) != viPred.end())
            {
                for (unsigned vk : viPred.at(EdgeLabelType::Store))
                {
                    if (!graph->hasEdge(vk, vj, EdgeLabelType::SV))
                    {
                        graph->addEdge(vk, vj, EdgeLabelType::SV);
                        workList.push(CFLREdge(vk, vj, EdgeLabelType::SV));
                    }
                }
            }
            
            // SV ::= VA Store
            if (X == EdgeLabelType::Store && viPred.find(EdgeLabelType::VA) != viPred.end())
            {
                for (unsigned vk : viPred.at(EdgeLabelType::VA))
                {
                    if (!graph->hasEdge(vk, vj, EdgeLabelType::SV))
                    {
                        graph->addEdge(vk, vj, EdgeLabelType::SV);
                        workList.push(CFLREdge(vk, vj, EdgeLabelType::SV));
                    }
                }
            }
            
            // PV ::= PT VA
            if (X == EdgeLabelType::VA && viPred.find(EdgeLabelType::PT) != viPred.end())
            {
                for (unsigned vk : viPred.at(EdgeLabelType::PT))
                {
                    if (!graph->hasEdge(vk, vj, EdgeLabelType::PV))
                    {
                        graph->addEdge(vk, vj, EdgeLabelType::PV);
                        workList.push(CFLREdge(vk, vj, EdgeLabelType::PV));
                    }
                }
            }
            
            // VP ::= VA PT
            if (X == EdgeLabelType::PT && viPred.find(EdgeLabelType::VA) != viPred.end())
            {
                for (unsigned vk : viPred.at(EdgeLabelType::VA))
                {
                    if (!graph->hasEdge(vk, vj, EdgeLabelType::VP))
                    {
                        graph->addEdge(vk, vj, EdgeLabelType::VP);
                        workList.push(CFLREdge(vk, vj, EdgeLabelType::VP));
                    }
                }
            }
            
            // LV ::= Load VA
            if (X == EdgeLabelType::VA && viPred.find(EdgeLabelType::Load) != viPred.end())
            {
                for (unsigned vk : viPred.at(EdgeLabelType::Load))
                {
                    if (!graph->hasEdge(vk, vj, EdgeLabelType::LV))
                    {
                        graph->addEdge(vk, vj, EdgeLabelType::LV);
                        workList.push(CFLREdge(vk, vj, EdgeLabelType::LV));
                    }
                }
            }
        }
    }
    
    std::cout << "CFL-Reachability algorithm completed after " 
              << iteration << " iterations." << std::endl;
}
