#pragma once

#include <iostream>
#include <render/structures.hpp>

namespace brep
{
    class solid;
    class face;
    class loop;
    class edge;
    class half_edge;
    class vertex;

    // 体
    class solid
    {
        public:
            solid()
                : prevs(nullptr), nexts(nullptr), sface(nullptr), edge(nullptr)
            {
            }

            solid* prevs;
            solid* nexts;
            face* sface; // 首面;
            edge* edge;  // 用于线框显示的边
    };

    // 面
    class face
    {
        public:
            face()
                : prevf(nullptr), nextf(nullptr), fsolid(nullptr),
                  floops(nullptr)
            {
            }

            face* prevf;
            face* nextf;
            solid* fsolid;
            loop* floops; // 首环
            int get_loop_count();
    };

    // 环
    class loop
    {
        public:
            loop()
                : prevl(nullptr), nextl(nullptr), lface(nullptr), ledge(nullptr)
            {
            }

            loop* prevl;
            loop* nextl;
            face* lface;
            half_edge* ledge; // 首边
    };

    // 半边
    class half_edge
    {
        public:
            half_edge()
                : prev(nullptr), next(nullptr), adjacent(nullptr),
                  startv(nullptr), endv(nullptr), hloop(nullptr), edge(nullptr)
            {
            }

            half_edge* prev;
            half_edge* next;
            half_edge* adjacent;
            vertex* startv;
            vertex* endv;
            loop* hloop;
            edge* edge; // 边
    };

    // 边
    class edge
    {
        public:
            edge()
                : preve(nullptr), nexte(nullptr), HalfEdge1(nullptr),
                  HalfEdge2(nullptr)
            {
            }

            edge* preve;
            edge* nexte;
            half_edge* HalfEdge1;
            half_edge* HalfEdge2;
    };

    // 顶点
    class vertex
    {
        public:
            vertex() : prevv(nullptr), nextv(nullptr), vedge(nullptr) {}
            render::point* point;
            vertex* prevv;
            vertex* nextv;
            half_edge* vedge;
    };

    inline int face::get_loop_count()
    {
        int count = 0;
        for (loop* current_loop = this->floops; current_loop != nullptr;
             current_loop = current_loop->nextl)
        {
            count++;
        }
        return count;
    }

} // namespace brep
