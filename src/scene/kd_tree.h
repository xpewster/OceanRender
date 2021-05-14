#ifndef KD_H
#define KD_H

#include "split.h"
#include "3d_rect.h"

#include <glm/glm.hpp>
#include <glm/gtx/string_cast.hpp>
#include <glm/gtx/norm.hpp>
#include <vector>
#include <string>
#include <iostream>
#include <algorithm>

template<typename T>
class Kd_node{
public:
    T data;
    glm::vec3 location;
    Kd_node* left;
    Kd_node* right;
    prism rect;

    Kd_node(T data, glm::vec3 location) :
        data(data), location(location){}
};

template<typename T>
struct cone_filter_data{
public:
    T* data;
    float distance;

    cone_filter_data(T* data, float distance) :
        data(data), distance(distance) {}
};

template<typename T>
class Kd_tree{
public:
    Kd_tree();
    ~Kd_tree();

    Kd_tree(std::vector<T>& points, float _bound) {
        for(const T p : points){
            nodes.emplace_back(p, glm::vec3(p[SPLIT_X], p[SPLIT_Y], p[SPLIT_Z]));
        }
        bound = _bound;
        root = Kd_tree_helper(0, nodes.size(), 0);
    }

    T find(glm::vec4 location) {return NULL;}
    void find_square(std::vector<cone_filter_data<T>>& result, glm::vec3 center, float w){
        find_square_helper({center, w, w, w}, result, root, {glm::vec3(0.0), bound, bound, bound}, 0);
    }

    void clear(){
        root = nullptr;
    }

    void print_tree(){
        print_tree_helper("", root, false);
    }

private:
    Kd_node<T>* root;
    std::vector<Kd_node<T>> nodes;
    float bound;

    Kd_node<T>* Kd_tree_helper(int o_begin, int o_end, int cur_depth){
        // if (points.size < 10){
        //     std::cout << "size of points vector less than 10" << std::endl;
        //     return;
        // }
        if (o_begin >= o_end)
            return nullptr;

        SPLIT_TYPE split = static_cast<SPLIT_TYPE>(cur_depth % (SPLIT_Z+1));

        /* Find median */
        // const int sample_size = (end-begin < 10) ? end-begin : 10;
        // std::vector<T*> sample; 
        // for(size_t i = 0; i < points.size(); i += points.size()/sample_size){
        //     sample.push_back(&points[i]);
        // }
        // i_sort(sample, split);
        // glm::vec4 median = glm::vec4(sample[sample.size()/2][SPLIT_X], sample[sample.size()/2][SPLIT_Y], sample[sample.size()/2][SPLIT_Z], 1.0f);

        int mid = (o_end+o_begin)/2;

        std::nth_element(nodes.begin() + o_begin, nodes.begin() + mid, nodes.begin() + o_end, [split](Kd_node<T>& a, Kd_node<T>& b){
            return a.location[split] < b.location[split];
        });

        /* Construct subtree */
        // Kd_node<T>* n = new Kd_node<T>(*sample[sample.size()/2], glm::vec3(median));
        // std::vector<T*> less, greater;
        // std::partition_copy(points.begin(), points.end(), less.begin(), greater.begin(), [glm::vec4& median, SPLIT_TYPE split](T*& p){
        //     return (*p)[split] < median[split];
        // }); 
        // n->left = Kd_tree_helper(less, depth+1);
        // n->right = Kd_tree_helper(greater, depth+1);
        nodes[mid].left = Kd_tree_helper(o_begin, mid, cur_depth+1);
        nodes[mid].right = Kd_tree_helper(mid+1, o_end, cur_depth+1);
        return &nodes[mid];
    }

    void find_square_helper(prism rect, std::vector<cone_filter_data<T>>& found, Kd_node<T>* curr, prism cell, int cur_depth){
        
        if (curr == nullptr)
            return;
        //std::cout << "Checking node: " << glm::to_string(curr->location) << " cell: " << glm::to_string(cell.center) << ", " << cell.w << ", " << cell.h << ", " << cell.d << std::endl;
        if (!rect.overlaps(cell)){
            // std::cout << "!overlaps\n";
            return;
        }
        if (rect.contains(cell)){
            // std::cout << "contains_all!\n";
            add_all_below(rect, found, curr);
        }
        else {
    
            if (rect.contains(curr->location)){
                // std::cout << "contains!\n";
                cone_filter_data<T> d(&curr->data, glm::distance2(curr->location, rect.center));
                found.push_back(d);
            }

            if (curr->left != nullptr || curr->right != nullptr){
                SPLIT_TYPE split = static_cast<SPLIT_TYPE>(cur_depth % (SPLIT_Z+1));
                
                glm::vec3 cell_world_off = cell.center - cell.offset(split);
                glm::vec3 node_cellspace_off = glm::vec3(0.0);
                node_cellspace_off[split] = curr->location[split] - cell_world_off[split];

                glm::vec3 lcenter = cell_world_off + node_cellspace_off/2.0f;
                float dim[SPLIT_Z+1];
                for(size_t i = 0; i < SPLIT_Z+1; i++){
                    if (i == split)
                        dim[i] = node_cellspace_off[i];
                    else
                        dim[i] = cell[i];
                }
                prism lcell(lcenter, dim[0], dim[1], dim[2]);

                glm::vec3 node_to_cell_end = glm::vec3(0.0);
                node_to_cell_end[split] = cell.center[split] + cell[split] - curr->location[split];
                glm::vec3 rcenter = cell_world_off + node_cellspace_off + node_to_cell_end/2.0f;
                for(size_t i = 0; i < SPLIT_Z+1; i++){
                    if (i == split)
                        dim[i] = node_to_cell_end[i];
                    else
                        dim[i] = cell[i];
                }
                prism rcell(rcenter, dim[0], dim[1], dim[2]);

                // std::cout << "lcell: " << glm::to_string(lcell.center) << ", " << lcell.w << ", " << lcell.h << ", " << lcell.d << std::endl;
                // std::cout << "rcell: " << glm::to_string(rcell.center) << ", " << rcell.w << ", " << rcell.h << ", " << rcell.d << std::endl << std::endl;
                find_square_helper(rect, found, curr->left, lcell, cur_depth+1);
                find_square_helper(rect, found, curr->right, rcell, cur_depth+1);
            }
        }
    }

    void add_all_below(prism rect, std::vector<cone_filter_data<T>>& vec, Kd_node<T>* curr){
        if (curr == nullptr)
            return;
        
        cone_filter_data<T> d(&curr->data, glm::distance2(curr->location, rect.center));
        vec.push_back(d);

        add_all_below(rect, vec, curr->left);
        add_all_below(rect, vec, curr->right);
    }



    void print_tree_helper(const std::string& prefix, Kd_node<T>* cur, bool is_left) {
        if( cur != nullptr )
        {
            std::cout << prefix;
            std::cout << (is_left ? "L------- " : "\\------- " );

            // print the value of the node
            std::cout << glm::to_string(cur->data.location) << std::endl;

            // enter the next tree level - left and right branch
            print_tree_helper( prefix + (is_left ? "|        " : "         "), cur->left, true);
            print_tree_helper( prefix + (is_left ? "|        " : "         "), cur->right, false);
        }
    }


};

#endif