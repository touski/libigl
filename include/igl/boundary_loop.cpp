// This file is part of libigl, a simple c++ geometry processing library.
// 
// Copyright (C) 2014 Stefan Brugger <stefanbrugger@gmail.com>
// 
// This Source Code Form is subject to the terms of the Mozilla Public License 
// v. 2.0. If a copy of the MPL was not distributed with this file, You can 
// obtain one at http://mozilla.org/MPL/2.0/.
#include "boundary_loop.h"
#include "slice.h"
#include "triangle_triangle_adjacency.h"
#include "vertex_triangle_adjacency.h"
#include "is_border_vertex.h"
#include <set>

template <typename DerivedF, typename Index>
IGL_INLINE void igl::boundary_loop(
    const Eigen::PlainObjectBase<DerivedF> & F, 
    std::vector<std::vector<Index> >& L)
{
  using namespace std;
  using namespace Eigen;
  using namespace igl;

  MatrixXd Vdummy(F.maxCoeff(),1);
  MatrixXi TT,TTi;
  vector<std::vector<int> > VF, VFi;
  triangle_triangle_adjacency(Vdummy,F,TT,TTi);
  vertex_triangle_adjacency(Vdummy,F,VF,VFi);

  vector<bool> unvisited = is_border_vertex(Vdummy,F);
  set<int> unseen;
  for (int i = 0; i < unvisited.size(); ++i)
  {
    if (unvisited[i])
      unseen.insert(unseen.end(),i);
  }

  while (!unseen.empty())
  {
    vector<Index> l;

    // Get first vertex of loop
    int start = *unseen.begin();
    unseen.erase(unseen.begin());
    unvisited[start] = false;
    l.push_back(start);

    bool done = false;
    while (!done)
    {
      // Find next vertex
      bool newBndEdge = false;
      int v = l[l.size()-1];
      int next;
      for (int i = 0; i < (int)VF[v].size() && !newBndEdge; i++)
      {
        int fid = VF[v][i];

        if (TT.row(fid).minCoeff() < 0.) // Face contains boundary edge
        {
          int vLoc;
          if (F(fid,0) == v) vLoc = 0;
          if (F(fid,1) == v) vLoc = 1;
          if (F(fid,2) == v) vLoc = 2;

          int vPrev = F(fid,(vLoc + F.cols()-1) % F.cols());
          int vNext = F(fid,(vLoc + 1) % F.cols());

          bool newBndEdge = false;
          if (unvisited[vPrev] && TT(fid,(vLoc+2) % F.cols()) < 0)
          {
            next = vPrev;
            newBndEdge = true;
          }
          else if (unvisited[vNext] && TT(fid,vLoc) < 0)
          {
            next = vNext;
            newBndEdge = true;
          }
        }
      }

      if (newBndEdge)
      {
        l.push_back(next);
        unseen.erase(next);
        unvisited[next] = false;
      }
      else
        done = true;
    }
    L.push_back(l);
  }
}

template <typename DerivedF, typename Index>
IGL_INLINE void igl::boundary_loop(
  const Eigen::PlainObjectBase<DerivedF>& F, 
  std::vector<Index>& L)
{
  using namespace Eigen;
  using namespace std;

  vector<vector<int> > Lall;
  boundary_loop(F,Lall);

  int idxMax = -1;
  int maxLen = 0;
  for (int i = 0; i < Lall.size(); ++i)
  {
    if (Lall[i].size() > maxLen)
    {
      maxLen = Lall[i].size();
      idxMax = i;
    }
  }   

  L.resize(Lall[idxMax].size());
  for (int i = 0; i < Lall[idxMax].size(); ++i)
    L[i] = Lall[idxMax][i];
}

template <typename DerivedF, typename DerivedL>
IGL_INLINE void igl::boundary_loop(
  const Eigen::PlainObjectBase<DerivedF>& F, 
  Eigen::PlainObjectBase<DerivedL>& L)
{
  using namespace Eigen;
  using namespace std;

  vector<int> Lvec;
  boundary_loop(F,Lvec);

  L.resize(Lvec.size());
  for (int i = 0; i < Lvec.size(); ++i)
    L(i) = Lvec[i];
}