#include <iostream>

#include "definitions.h"
#include "checkers_network.h"
#include "logger.h"
#include "self_checksum.h"
#include "snippet_inserter.h"

#include "BPatch.h"
#include "BPatch_basicBlock.h"

#include <list>

namespace selfchecksum {

namespace {

void insert_snippets(const std::string& binary_name,
                     BPatch_binaryEdit* binary,
                     const logger& log,
                     checkers_network& network)
{
    snippet_inserter inserter(binary_name, binary, log);

    auto& leaves = network.get_leaves();
    auto& nodes = network.get_nodes();
    std::list<checkers_network::node_type> checkers_queue;
    checkers_queue.insert(checkers_queue.begin(), leaves.begin(), leaves.end());

    for (auto& node : nodes) {
        BPatch_basicBlock* node_block = node->get_block();
        unsigned long long node_id = node->get_order_id();
        inserter.insertEndCheckTag(node_block, node_id);
    }
        
    for (auto& node : nodes) {
        BPatch_basicBlock* node_block = node->get_block();
        unsigned long long node_id = node->get_order_id();
        auto& checkers = node->get_checkers();
        for (auto& checker : checkers) {
            inserter.insertAddrHash(checker->get_block(), node_block, node_id, checker->checks_only_block(node_block));
        }
    }
    for (auto& node : nodes) {
        BPatch_basicBlock* node_block = node->get_block();
        unsigned long long node_id = node->get_order_id();
        inserter.insertBlockTag(node_block, node_id);
    }

/*
    while (!checkers_queue.empty()) {
        auto leaf = checkers_queue.back();
        printf("%llu\n", leaf->get_order_id());
        checkers_queue.pop_back();
        BPatch_basicBlock* leaf_block = leaf->get_block();
        //std::cout << "Block order: " << leaf->get_order_id() << " block number " << leaf->get_block()->getBlockNumber() << "\n";
        auto& checkers = leaf->get_checkers();
        for (auto& checker : checkers) {
            inserter.insertAddrHash(checker->get_block(), leaf_block, leaf->get_order_id(), checker->checks_only_block(leaf_block));
            checker->remove_checkee(leaf_block);
            if (!checker->has_checkees()) {
                checkers_queue.push_front(checker);
            }
        }
    }
    for (auto& node : nodes) {
        inserter.insertBlockTag(node->get_block(), node->get_order_id());
    }
*/
}

}

void self_checksum::run(const std::string& binary_name, unsigned connectivity)
{
    logger log;
    BPatch bpatch;
    BPatch_binaryEdit* binary = bpatch.openBinary(binary_name.c_str(), true);
    BPatch_image* appImg = binary->getImage();
    if (appImg == nullptr) {
        log.log_error("No binary image found\n");
        return;
    }
    auto modules = appImg->getModules();
    if (modules == nullptr) {
        log.log_error("No modules found\n");
        return;
    }
    checkers_network network(*modules, connectivity, log);
    printf("Building network\n");
    network.build();
    printf("Dumping network\n");
    //network.dump(binary_name);
    printf("Inserting Snippets\n");
    insert_snippets(binary_name, binary, log, network);
}

void self_checksum::run(const std::string& binary_name, const std::string& module_name, unsigned connectivity)
{
    logger log;
    BPatch bpatch;
    BPatch_binaryEdit* binary = bpatch.openBinary(binary_name.c_str(), true);
    BPatch_image* appImg = binary->getImage();
    if (appImg == nullptr) {
        log.log_error("No binary image found\n");
        return;
    }
    BPatch_module* module = appImg->findModule(module_name.c_str(), false);
    if (module == nullptr) {
        log.log_error("No module found\n");
        return;
    }
    checkers_network network(module, connectivity, log);
    printf("Building network\n");
    network.build();
    printf("Dumping network\n");
    network.dump(binary_name);
    printf("Inserting Snippets\n");
    insert_snippets(binary_name, binary, log, network);
}

}

int main(int argc, char* argv[])
{
    if (argc < 3) {
        std::cerr << "Wrong number of arguments\n";
        return 1;
    }
    selfchecksum::self_checksum checksum;
    std::string binary_name(argv[1]);
    unsigned connectivity = atoi(argv[2]);
    if (argc == 4) {
        std::string module_name(argv[3]);
        checksum.run(binary_name, module_name, connectivity);
    } else {
        checksum.run(binary_name, connectivity);
    }

    //std::string mesage("Creating network graph for executable: " + binary_name
    //                + ", module: " + module_name
    //                + ", connectivity level: " + std::to_string(connectivity));

    //std::cout << mesage << "\n";
    //const std::string binary_name("/home/anahitik/TUM_S17/SIP/Introspection/self-checksumming/tests/test");
    //const std::string module_name("test");
    //unsigned connectivity = 2;

    return 0;
}

