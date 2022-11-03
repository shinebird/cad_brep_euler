#include "render/structures.hpp"
#include <b-rep.hpp>

#include <format>

namespace brep
{
    solid* mvfs(render::point* point, vertex** new_vertex)
    {
        // 建立点、面、体、环
        solid* new_solid = new solid;
        face* new_face = new face;
        loop* new_loop = new loop;

        auto vertex_ptr = new vertex;
        *new_vertex = vertex_ptr;
        // 记录初始点的几何信息
        vertex_ptr->point = point;

        // // 构建拓扑关系
        new_solid->sface = new_face;
        new_face->fsolid = new_solid;
        new_face->floops = new_loop;
        new_loop->lface = new_face;
        new_loop->ledge = nullptr;
        return new_solid;
    }

    half_edge* mev(vertex* v1, render::point* p2, loop* lp)
    {
        solid* solid = lp->lface->fsolid;
        half_edge* he1 = new half_edge;
        half_edge* he2 = new half_edge;
        half_edge* he = new half_edge;
        edge* edge = new brep::edge;

        vertex* v2 = new vertex;
        v2->point = p2;

        he1->edge = he2->edge = edge;
        edge->HalfEdge1 = he1;
        edge->HalfEdge2 = he2;

        he1->hloop = he2->hloop = lp;
        he1->startv = v1;
        he1->endv = v2;
        he2->startv = v2;
        he2->endv = v1;

        he1->adjacent = he2;
        he2->adjacent = he1;

        if (lp->ledge == nullptr)
        { // First create edge
            he1->next = he2;
            he2->prev = he1;
            he2->next = he1;
            he1->prev = he2;
            lp->ledge = he1;
        }
        else
        { // Following create edges
            for (he = lp->ledge; he->next->startv != v1; he = he->next)
                ;
            he1->next = he2;
            he1->prev = he;
            he2->next = he->next;
            he2->prev = he1;
            he->next->prev = he2;
            he->next = he1;
        }

        // Maintain Edge List
        brep::edge* curEdge = solid->edge;
        while (curEdge != nullptr && curEdge->nexte != nullptr)
        {
            curEdge = curEdge->nexte;
        }
        if (curEdge == nullptr)
            solid->edge = edge;
        else
        {
            curEdge->nexte = edge;
            edge->preve = curEdge;
        }

        return he1;
    }

    loop* mef(vertex* v1, vertex* v2, loop* lp)
    {
        solid* solid = lp->lface->fsolid;
        face* face = new brep::face;
        loop* loop = new brep::loop;
        half_edge* he1 = new brep::half_edge;
        half_edge* he2 = new brep::half_edge;
        edge* edge = new brep::edge;

        // Create half_edge and Edge
        he1->startv = v1;
        he1->endv = v2;
        he2->startv = v2;
        he2->endv = v1;
        he1->adjacent = he2;
        he2->adjacent = he1;

        edge->HalfEdge1 = he1;
        edge->HalfEdge2 = he2;
        he1->edge = he2->edge = edge;

        // Find the half_edge
        brep::half_edge *tmp_he1, *tmp_he2, *tmp_he;
        for (tmp_he = lp->ledge; tmp_he->startv != v1; tmp_he = tmp_he->next)
            ;
        tmp_he1 = tmp_he;

        for (; tmp_he->startv != v2; tmp_he = tmp_he->next)
            ;
        tmp_he2 = tmp_he;
        tmp_he = tmp_he->next;
        while (tmp_he->startv != v2)
        {
            tmp_he = tmp_he->next;
        }
        // bool HaveRoll = false;
        if (tmp_he != tmp_he2)
        {
            // HaveRoll = true;
            tmp_he2 = tmp_he;
        }

        // for (tmphe2 = lp->ledge; tmphe2->endv != v2; tmphe2 = tmphe2->next);

        //  he1->next = tmphe2->next;
        //  he1->prev = tmphe1;
        //  he2->next = tmphe1->next;
        //  he2->prev = tmphe2;
        //  tmphe1->next->prev = he2;
        //  tmphe1->next = he1;
        //  tmphe2->next->prev = he1;
        //  tmphe2->next = he2;

        he1->next = tmp_he2;
        he1->prev = tmp_he1->prev;
        he2->next = tmp_he1;
        he2->prev = tmp_he2->prev;

        tmp_he1->prev->next = he1;
        tmp_he1->prev = he2;

        tmp_he2->prev->next = he2;
        tmp_he2->prev = he1;

        // loop have problem
        ////////////////////////////////////
        // Inner loop
        loop->ledge = he1;
        // Recur loop
        lp->ledge = he2;

        // Create A new face
        face->floops = loop;
        face->fsolid = solid;
        loop->lface = face;
        // Add face to solid
        brep::face* tmp_face;
        for (tmp_face = solid->sface; tmp_face->nextf != nullptr;
             tmp_face = tmp_face->nextf)
            ;
        tmp_face->nextf = face;
        face->prevf = tmp_face;
        face->fsolid = solid;

        // Maintain Edge List
        brep::edge* current_edge = solid->edge;
        while (current_edge != nullptr && current_edge->nexte != nullptr)
        {
            current_edge = current_edge->nexte;
        }
        if (current_edge == nullptr)
            solid->edge = edge;
        else
        {
            current_edge->nexte = edge;
            edge->preve = current_edge;
        }

        return loop;
    }

    loop* kemr(vertex* v1, vertex* v2, loop* lp)
    {
        brep::face* face = lp->lface;
        brep::solid* solid = face->fsolid;
        brep::loop* loop = new brep::loop;

        brep::half_edge* he;
        // find the half edge
        for (he = lp->ledge; (he->startv != v2) || (he->endv != v1);
             he = he->next)
            ;

        // get related edge
        brep::edge* edge = he->edge;

        // create loop relation
        he->next->prev = he->adjacent->prev;
        he->adjacent->prev->next = he->next;

        he->prev->next = he->adjacent->next;
        he->adjacent->next->prev = he->prev;

        lp->ledge = he->next->prev;

        loop->ledge = he->prev;
        loop->lface = face;

        // insert the inner loop
        if (face->floops == nullptr)
            face->floops = loop;
        else
        {
            brep::loop* tmp_loop;
            for (tmp_loop = face->floops; tmp_loop->nextl != nullptr;
                 tmp_loop = tmp_loop->nextl)
                ;
            tmp_loop->nextl = loop;
            loop->prevl = tmp_loop;
        }

        // find edge will delete
        brep::edge* tmp_edge = solid->edge;
        while (tmp_edge != edge)
        {
            tmp_edge = tmp_edge->nexte;
        }
        // delete edge
        if (tmp_edge->nexte == nullptr)
        {
            tmp_edge->preve->nexte = nullptr;
        }
        else if (tmp_edge->preve == nullptr)
        {
            solid->edge = tmp_edge->nexte;
            tmp_edge->nexte->preve = nullptr;
        }
        else
        {
            tmp_edge->preve->nexte = tmp_edge->nexte;
            tmp_edge->nexte->preve = tmp_edge->preve;
        }
        delete tmp_edge;

        return loop;
    }

    void kfmrh(loop* out_lp, loop* lp)
    {
        brep::face* face1 = out_lp->lface;
        brep::face* face2 =
            lp->lface; // 要删去的顶面 的环，将它降到底面当内环，删去这个面。
        int face2_loop_count = face2->get_loop_count();
        if (face2_loop_count > 1)
        {
            std::cout
                << std::format(
                       "[ERROR] delete face has more than one loop ({} loops)",
                       face2_loop_count)
                << std::endl;

            return;
        }
        // add inner loop to face
        if (face1->floops == nullptr)
            face1->floops = lp;
        else
        {
            brep::loop* tmp_loop;
            for (tmp_loop = face1->floops; tmp_loop->nextl != nullptr;
                 tmp_loop = tmp_loop->nextl)
                ;
            tmp_loop->nextl = lp;
            lp->prevl = tmp_loop;
        }
        lp->lface = face1;

        // Find the face to remove
        brep::solid* solid = face1->fsolid;
        brep::face* tmpFace = solid->sface;
        while (tmpFace != face2)
        {
            tmpFace = tmpFace->nextf;
        }
        // delete Face
        if (tmpFace->nextf == nullptr)
        {
            tmpFace->prevf->nextf = nullptr;
        }
        else if (tmpFace->prevf == nullptr)
        {
            solid->sface = tmpFace->nextf;
            tmpFace->nextf->prevf = nullptr;
        }
        else
        {
            tmpFace->prevf->nextf = tmpFace->nextf;
            tmpFace->nextf->prevf = tmpFace->prevf;
        }
        delete tmpFace;
    }

    void sweep(face* face, double vec[3], double distance)
    {
        brep::loop* loop;
        brep::half_edge* he;
        brep::vertex* first_v;
        brep::vertex* up_vertex;
        brep::vertex* prev_up_vertex;
        brep::vertex* next_v;
        render::point* up_point;
        brep::half_edge* up_he;
        brep::half_edge* first_up_he;

        for (loop = face->floops; loop != nullptr; loop = loop->nextl)
        {
            he = loop->ledge;
            first_v = he->startv;
            vcl::Vec4d first_v_coord_vec;
            vcl::Vec4d d_vec;
            first_v_coord_vec.load_partial(3, first_v->point->coord);
            d_vec.load_partial(3, vec);
            auto result_vec = first_v_coord_vec + distance * d_vec;
            double result[3];
            result_vec.store_partial(3, result);
            up_point = new render::point(result);
            first_up_he = mev(first_v, up_point, loop);
            prev_up_vertex = first_up_he->endv;
            he = he->next;
            next_v = he->startv;
            while (next_v != first_v)
            {
                vcl::Vec4d next_v_coord_vec;
                vcl::Vec4d d_vec_1;
                next_v_coord_vec.load_partial(3, next_v->point->coord);
                d_vec_1.load_partial(3, vec);
                auto result_vec_1 = next_v_coord_vec + distance * d_vec_1;
                double result_1[3];
                result_vec_1.store_partial(3, result_1);
                up_point = new render::point(result_1);
                up_he = mev(next_v, up_point, loop);
                up_vertex = up_he->endv;
                mef(up_vertex, prev_up_vertex, loop);
                prev_up_vertex = up_vertex;
                he = he->next;
                next_v = he->startv;
            }
            mef(first_up_he->endv, prev_up_vertex, loop);
        }
    }
} // namespace brep
