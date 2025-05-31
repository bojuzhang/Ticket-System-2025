#pragma once
#include <iostream>
#ifndef BPT_HPP
#define BPT_HPP

#include <climits>
#include <string>
#include <filesystem>
#include "mystl.hpp"

using string64 = sjtu::MyString<64>;
using sjtu::MemoryRiver;
using sjtu::pair;
using sjtu::vector;

template <class TKey, class TValue>
class BPlusTree {
private:
    static const int kORDER = ((4096 * 8 - 10) / (sizeof(TKey) + sizeof(TValue) + 4) - 1) / 2 * 2 + 1; 
    static constexpr int kMAX_KEYS = kORDER - 1;
    static constexpr int kMIN_KEYS = (kORDER + 1) / 2 - 1;
    using kv_type = pair<TKey, TValue>;

    struct Node {
        bool isleaf;
        int keycount;
        kv_type kvs[kMAX_KEYS + 2]{}; // first key, second value
        int children[kORDER + 2];
        int next;

        Node(bool leaf = true) : isleaf(leaf), keycount(0), next(-1) {
            for (int i = 0; i < kORDER; i++)
                children[i] = -1;
        }
    };
    MemoryRiver<Node> file;
    int rootpos;

    static constexpr int kCACHESIZE = 128;
    struct Cache {
        bool used;
        int pos;
        Node node;
        int lstuse;
        Cache() : used(0), pos(-1), lstuse(0) {};
    } cache[kCACHESIZE];
    int usecnt = 1;

public:
    BPlusTree(const std::string &filename) {
        // std::cerr << "test " << filename << " " << kORDER << "\n";
        if (!std::filesystem::exists(filename)) {
            file.initialise(filename, 1);
            Node root;
            rootpos = AddNode(root);
        } else {
            file.initialise(filename, 1);
            file.get_info(rootpos, 2);
            Node root;
            root = ReadNode(rootpos);
        }
    }
    ~BPlusTree() {
        Node root;
        root = ReadNode(rootpos);
        file.write_info(rootpos, 2);
    }

    void Clear() {
        usecnt = 0;
        for (int i = 0; i < kCACHESIZE; i++) {
            cache[i].used = 0;
        }
        file.clear();
        Node root;
        rootpos = AddNode(root);
    }

    void Insert(const TKey &key, const TValue &value) {
        Node root = ReadNode(rootpos);
        if (root.isleaf && root.keycount == 0) {
            root.kvs[0] = {key, value};
            root.keycount = 1;
            WriteNode(rootpos, root);
            root = ReadNode(rootpos);
            return;
        }
        auto [newkvs, newpos] = InsertRec(rootpos, key, value);
        if (newpos != -1) {
            Node newroot(false);
            newroot.keycount = 1;
            newroot.kvs[0] = newkvs;
            newroot.children[0] = rootpos;
            newroot.children[1] = newpos;
            rootpos = AddNode(newroot);
        }
    }

    pair<kv_type, int> InsertRec(int pos, const TKey &key, const TValue &value) {
        Node node = ReadNode(pos);
        if (node.isleaf) {
            int i = 0;
            while (i < node.keycount && node.kvs[i] < pair{key, value}) {
                i++;
            }
            // std::cerr << "   InsertRec: " << node.keycount << " " << i << "\n";
            if (i < node.keycount && node.kvs[i] == pair{key, value})
                return {kv_type(), -1};
            for (int k = node.keycount; k > i; k--) {
                node.kvs[k] = node.kvs[k - 1];
            }
            node.kvs[i] = {key, value};
            node.keycount++;
            if (node.keycount <= kMAX_KEYS) {
                WriteNode(pos, node);
                return {kv_type(), -1};
            }
            int mid = node.keycount / 2;
            Node newleaf;
            newleaf.isleaf = true;
            newleaf.keycount = node.keycount - mid;
            for (int k = 0; k < newleaf.keycount; k++) {
                newleaf.kvs[k] = node.kvs[mid + k];
            }
            node.keycount = mid;
            newleaf.next = node.next;
            int newpos = AddNode(newleaf);
            node.next = newpos;
            WriteNode(pos, node);
            return {newleaf.kvs[0], newpos};
        } else {
            int i = 0;
            while (i < node.keycount && node.kvs[i] < pair{key, value}) {
                i++;
            }
            auto [newkvs, newchild] = InsertRec(node.children[i], key, value);
            if (newchild == -1) {
                return {kv_type(), -1};
            }
            for (int k = node.keycount; k > i; k--) {
                node.kvs[k] = node.kvs[k - 1];
                node.children[k + 1] = node.children[k];
            }
            node.kvs[i] = newkvs;
            node.children[i + 1] = newchild;
            node.keycount++;
            if (node.keycount <= kMAX_KEYS) {
                WriteNode(pos, node);
                return {kv_type(), -1};
            }
            int mid = node.keycount / 2;
            Node newnode(false);
            newnode.keycount = node.keycount - mid - 1;
            for (int k = 0; k < newnode.keycount; k++) {
                newnode.kvs[k] = node.kvs[mid + 1 + k];
                newnode.children[k] = node.children[mid + 1 + k];
            }
            newnode.children[newnode.keycount] = node.children[node.keycount];
            kv_type promote = node.kvs[mid];
            node.keycount = mid;
            WriteNode(pos, node);
            int newpos = AddNode(newnode);
            return {promote, newpos};
        }
    }

    void Remove(const TKey &key, const TValue &value) {
        RemoveRec(rootpos, pair{key, value});
    }

    bool RemoveRec(int pos, const kv_type &item, int parent = -1, int idx = -1, const Node &p = Node(false)) {
        Node node = ReadNode(pos);
        if (node.isleaf) {
            int i = 0;
            while (i < node.keycount && node.kvs[i] < item) {
                i++;
            }
            if (i == node.keycount || node.kvs[i] != item) {
                return false;
            }
            for (int k = i; k < node.keycount - 1; k++) {
                node.kvs[k] = node.kvs[k + 1];
            }
            node.keycount--;
            WriteNode(pos, node);
            if (node.keycount < kMIN_KEYS) {
                RebalanceLeaf(parent, idx, pos, p, node);
            }
            return true;
        } else {
            int i = 0;
            while (i < node.keycount && node.kvs[i] <= item) {
                i++;
            }
            bool merged = RemoveRec(node.children[i], item, pos, i, node);
            if (!merged) {
                return false;
            }
            if (pos == rootpos) {
                return false;
            }
            return true;
        }
    }

    void RebalanceLeaf(int parent, int idx, int pos, Node p, Node now) {
        if (parent == -1) return;
        int left = (idx > 0 ? p.children[idx - 1] : -1),
            right = (idx < p.keycount ? p.children[idx + 1] : -1);
        if (left != -1) {
            Node ls = ReadNode(left);
            if (ls.keycount > kMIN_KEYS) {
                for (int k = now.keycount; k > 0; k--) {
                    now.kvs[k] = now.kvs[k - 1];
                }
                now.kvs[0] = ls.kvs[ls.keycount - 1];
                now.keycount++;
                ls.keycount--;
                WriteNode(left, ls);
                WriteNode(pos, now);
                p.kvs[idx - 1] = now.kvs[0];
                WriteNode(parent, p);
                return;
            }
        }
        if (right != -1) {
            Node rs = ReadNode(right);
            if (rs.keycount > kMIN_KEYS) {
                now.kvs[now.keycount] = rs.kvs[0];
                now.keycount++;
                for (int k = 0; k < rs.keycount - 1; k++) {
                    rs.kvs[k] = rs.kvs[k + 1];
                }
                rs.keycount--;
                WriteNode(right, rs);
                WriteNode(pos, now);
                p.kvs[idx] = rs.kvs[0];
                WriteNode(parent, p);
                return;
            }
        }
        if (left != -1) {
            MergeNode(parent, idx - 1, idx);
        } else if (right != -1) {
            MergeNode(parent, idx, idx);
        }
    }

    void RebalanceInternal(int parentpos, int idx, Node p, Node now) {
        if (parentpos == -1) return;
        int pos = p.children[idx];
        int left = (idx > 0 ? p.children[idx - 1] : -1);
        int right = (idx < p.keycount ? p.children[idx + 1] : -1);
        if (left != -1) {
            Node ls = ReadNode(left);
            if (ls.keycount > kMIN_KEYS) {
                for (int k = now.keycount; k > 0; k--) {
                    now.kvs[k] = now.kvs[k - 1];
                }
                now.kvs[0] = p.kvs[idx - 1];
                for (int k = now.keycount + 1; k > 0; k--) {
                    now.children[k] = now.children[k - 1];
                }
                now.children[0] = ls.children[ls.keycount];
                now.keycount++;
                p.kvs[idx - 1] = ls.kvs[ls.keycount - 1];
                ls.keycount--;
                WriteNode(left, ls);
                WriteNode(pos, now);
                WriteNode(parentpos, p);
                return;
            }
        }
        if (right != -1) {
            Node rs = ReadNode(right);
            if (rs.keycount > kMIN_KEYS) {
                now.kvs[now.keycount] = p.kvs[idx];
                now.children[now.keycount + 1] = rs.children[0];
                now.keycount++;
                p.kvs[idx] = rs.kvs[0];
                for (int k = 0; k < rs.keycount - 1; k++) {
                    rs.kvs[k] = rs.kvs[k + 1];
                    rs.children[k] = rs.children[k + 1];
                }
                rs.children[rs.keycount - 1] = rs.children[rs.keycount];
                rs.keycount--;
                WriteNode(right, rs);
                WriteNode(pos, now);
                WriteNode(parentpos, p);
                return;
            }
        }
        if (left != -1) {
            MergeNode(parentpos, idx - 1, idx);
        } else if (right != -1) { 
            MergeNode(parentpos, idx, idx);
        }
    }

    void MergeNode(int parentpos, int idx, int need_del) {
        Node p = ReadNode(parentpos);
        if (p.keycount <= 0) return;
        int left = p.children[idx], right = p.children[idx + 1];
        Node ls = ReadNode(left), rs = ReadNode(right);
        if (ls.isleaf) {
            for (int k = 0; k < rs.keycount; k++) {
                ls.kvs[ls.keycount + k] = rs.kvs[k];
            }
            ls.keycount += rs.keycount;
            ls.next = rs.next;
        } else {
            ls.kvs[ls.keycount] = p.kvs[idx];
            ls.keycount++;
            for (int k = 0; k < rs.keycount; k++) {
                ls.kvs[ls.keycount + k] = rs.kvs[k];
            }
            for (int k = 0; k <= rs.keycount; k++) {
                ls.children[ls.keycount + k] = rs.children[k];
            }
            ls.keycount += rs.keycount;
        }
        WriteNode(left, ls);
        for (int k = idx; k < p.keycount - 1; k++) {
            p.kvs[k] = p.kvs[k + 1];
        }
        for (int k = idx + 1; k < p.keycount; k++) {
            p.children[k] = p.children[k + 1];
        }
        p.keycount--;
        WriteNode(parentpos, p);
        if (parentpos == rootpos && !p.keycount) {
            rootpos = left;
            return;
        }
        if (parentpos != rootpos && p.keycount < kMIN_KEYS) {
            auto [gp, gidx] = FindParent(rootpos, parentpos);
            RebalanceInternal(gp, gidx, ReadNode(gp), p);
        }
    }

    pair<int, int> FindParent(int current, int childPos) {
        Node node = ReadNode(current);
        if (node.isleaf) {
            return {-1, -1};
        }
        for (int i = 0; i <= node.keycount; i++) {
            if (node.children[i] == childPos) {
                return {current, i};
            }
            auto p = FindParent(node.children[i], childPos);
            if (p.first != -1) {
                return p;
            }
        }
        return {-1, -1};
    }

    vector<TValue> Find(const TKey &key) {
        int pos = FindLeaf(rootpos, key);
        vector<TValue> ans;
        while (1) {
            if (pos == -1) {
                break;
            }
            Node node = ReadNode(pos);
            if (node.keycount == 0 || node.kvs[0].first > key) {
                break;
            }
            bool first = true;
            for (int i = 0; i < node.keycount; i++) {
                if (node.kvs[i].first == key) {
                    ans.push_back(node.kvs[i].second);
                }
            }
            pos = node.next;
        }
        return ans;
    }
    int FindLeaf(int pos, const TKey &key) {
        Node node = ReadNode(pos);
        if (node.isleaf) {
            return pos;
        }
        int i = 0;
        while (i < node.keycount && node.kvs[i].first < key) {
            i++;
        }
        return FindLeaf(node.children[i], key);
    }

    Node ReadNode(int pos) {
        int minuse = INT_MAX;
        int p = -1;
        for (int i = 0; i < kCACHESIZE; i++) {
            if (cache[i].used && cache[i].pos == pos) {
                cache[i].lstuse = usecnt++;
                return cache[i].node;
            }
            if (!cache[i].used) {
                p = i;
                break;
            }
            if (cache[i].lstuse < minuse) {
                minuse = cache[i].lstuse;
                p = i;
            }
        }
        Node node;
        file.read(node, pos);
        if (p != -1) {
            cache[p].used = 1;
            cache[p].pos = pos;
            cache[p].node = node;
            cache[p].lstuse = usecnt++;
        }
        return node;
    }
    void WriteNode(int pos, Node &node) {
        int p = -1;
        for (int i = 0; i < kCACHESIZE; i++) {
            if (cache[i].used && cache[i].pos == pos) {
                p = i;
                break;
            }
        }
        if (p != -1) {
            cache[p].used = 1;
            cache[p].pos = pos;
            cache[p].node = node;
            cache[p].lstuse = usecnt++;
        }
        file.update(node, pos);
    }
    int AddNode(Node &p) { return file.write(p); }

    vector<TValue> Allvalues() {
        vector<TValue> ans;
        int pos = rootpos;
        Node node = ReadNode(pos);
        while (!node.isleaf) {
            pos = node.children[0];
            node = ReadNode(pos);
        }
        while (pos >= 0) {
            node = ReadNode(pos);
            for (int i = 0; i < node.keycount; i++) {
                ans.push_back(node.kvs[i].second);
            }
            pos = node.next;
        }
        return ans;
    }

    // void printTree() {
    //     bool used[100000];
    //     for (int i = 0; i <= 1000; i++)
    //         used[i] = 0;
    //     // std::cerr << "fjdifj " << ReadNode(rootpos).keycount << "\n";
    //     cout << "B+ Tree (levels):\n";
    //     // 使用队列进行层序遍历
    //     struct Item { long pos; int level; };
    //     vector<std::string> levels;
    //     std::queue<Item> q;
    //     q.push({rootpos, 0});
    //     while (!q.empty()) {
    //         auto it = q.front(); q.pop();
    //         Node node = ReadNode(it.pos);
    //         // 确保 levels 向量长度足够
    //         if (it.level >= (int)levels.size()) levels.push_back("");
    //         // 格式化当前节点
    //         std::string repr = node.isleaf ? "(" : "<";
    //         for (int i = 0; i < node.keycount; i++) {
    //             repr += std::string(node.kvs[i].first) + "," + std::to_string(node.kvs[i].second);
    //             if (i + 1 < node.keycount) repr += ";";
    //         }
    //         repr += ": " + std::to_string(it.pos);
    //         repr += node.isleaf ? ")" : ">";
    //         if (!levels[it.level].empty()) levels[it.level] += " | ";
    //         levels[it.level] += repr;
    //         // enqueue children
    //         if (!node.isleaf) {
    //             for (int i = 0; i <= node.keycount; i++) {
    //                 int x = node.children[i];
    //                 if (x == 86) {
    //                     std::cerr << "test: " << it.pos << " " << x << endl;
    //                 }
    //                 if (used[x]) {
    //                     std::cerr << "testdfjdfhj " << it.pos << " " << x << "\n";
    //                 } else {
    //                     used[x] = 1;
    //                 }
    //                 q.push({node.children[i], it.level + 1});
    //             }
    //         }
    //     }
    //     // 输出每一层
    //     for (int i = 0; i < (int)levels.size(); i++) {
    //         cout << "Level " << i << ": " << levels[i] << "\n";
    //     }
    // }
};

#endif // BPT_HPP