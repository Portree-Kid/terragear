// tgconstruct_poly.cxx -- load and handle polygon data
//
// Written by Curtis Olson, started May 1999.
//
// Copyright (C) 1999  Curtis L. Olson  - http://www.flightgear.org/~curt
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License as
// published by the Free Software Foundation; either version 2 of the
// License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful, but
// WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA.
//
// $Id: construct.cxx,v 1.4 2004-11-19 22:25:49 curt Exp $

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <boost/foreach.hpp>

#include <simgear/misc/sg_dir.hxx>
#include <simgear/debug/logstream.hxx>
#include <simgear/timing/timestamp.hxx>

// #include <terragear/tg_shapefile.hxx>

#include "tgconstruct.hxx"

int TGConstruct::LoadLandclassPolys( void )
{
    std::string base  = bucket.gen_base_path();
    std::string index = bucket.gen_index_str(); 
    std::string poly_path;
    tgPolygonSetList polys;
    //unsigned int total_polys_read = 0;
    
    // load 2D polygons from correct path
    poly_path = work_base + "/" + base + '/' + index;
    simgear::Dir d(poly_path);
    if (d.exists() ) {    
        simgear::PathList files = d.children(simgear::Dir::TYPE_FILE);
        SG_LOG( SG_CLIPPER, SG_INFO, files.size() << " Files in " << d.path() );
        
        BOOST_FOREACH(const SGPath& p, files) {
            std::string lext = p.complete_lower_extension();

            // look for .shp files to load
            if (lext == "shp") {
                SG_LOG(SG_GENERAL, SG_INFO, "load: " << p);
                
                // shapefile contains multiple polygons.
                // read an array of them
                tgPolygonSet::fromShapefile( p, polys );
                
                SG_LOG(SG_GENERAL, SG_INFO, "tgShapefile::ToPolygons returned " << polys.size() << " polygons" );
                
                for ( unsigned int i=0; i<polys.size(); i++ ) {
                    std::string material = polys[i].getMeta().getMaterial();

                    SG_LOG(SG_GENERAL, SG_INFO, "tgShapefile::ToPolygons poly " << i << " material is " << material );

                    int area = area_defs.get_area_priority( material );

                    SG_LOG(SG_GENERAL, SG_INFO, "tgShapefile::ToPolygons poly " << i << " area is " << area );
                    
                    tileMesh.addPoly( area, polys[i] );
                }
            }
        }
        
        if ( !tileMesh.empty() ) {
            // add pcean polygon
            addOceanPoly();
        }
    }
    
    return 0;
}

#define CORRECTION  (0.0005)
void TGConstruct::addOceanPoly( void )
{
    // set up clipping tile
    cgalPoly_Point pt[4];
    
    pt[0] = cgalPoly_Point( bucket.get_corner( SG_BUCKET_SW ).getLongitudeDeg()-CORRECTION, bucket.get_corner( SG_BUCKET_SW ).getLatitudeDeg()-CORRECTION );
    pt[1] = cgalPoly_Point( bucket.get_corner( SG_BUCKET_SE ).getLongitudeDeg()+CORRECTION, bucket.get_corner( SG_BUCKET_SE ).getLatitudeDeg()-CORRECTION );
    pt[2] = cgalPoly_Point( bucket.get_corner( SG_BUCKET_NE ).getLongitudeDeg()+CORRECTION, bucket.get_corner( SG_BUCKET_NE ).getLatitudeDeg()+CORRECTION );
    pt[3] = cgalPoly_Point( bucket.get_corner( SG_BUCKET_NW ).getLongitudeDeg()-CORRECTION, bucket.get_corner( SG_BUCKET_NW ).getLatitudeDeg()+CORRECTION );    
    
    cgalPoly_Polygon poly( pt, pt+4 );
    tgPolygonSetMeta meta(tgPolygonSetMeta::META_TEXTURED, area_defs.get_ocean_area_name() );
    
    tileMesh.addPoly( area_defs.get_ocean_area_priority(), tgPolygonSet( poly, meta ) );
}

#if 0
// when do we add the nodes when using CGAL? TBD
// maybe just keep a list of the fixed elevation nodes.
int TGConstruct::LoadLandclassPolys( void ) {
    int i;
    
    std::string base  = bucket.gen_base_path();
    std::string index = bucket.gen_index_str(); 
    std::string poly_path;
    unsigned int total_polys_read = 0;
    //tgPolygon poly;
    //tgpolygon_list polys;
    
    SGTimeStamp addnode;
    SGTimeStamp start, end;
    
    // load 2D polygons from all directories provided
    poly_path = work_base + "/" + base + '/' + index;
    
    simgear::Dir d(poly_path);
    if (d.exists() ) {    
        simgear::PathList files = d.children(simgear::Dir::TYPE_FILE);
        SG_LOG( SG_CLIPPER, SG_INFO, files.size() << " Files in " << d.path() );
        
        BOOST_FOREACH(const SGPath& p, files) {
            //if (p.file_base() != tile_str) {
            //    continue;
            //}
            
            string lext = p.complete_lower_extension();

            // look for .shp files to load
            if (lext == "shp") {
                SG_LOG(SG_GENERAL, SG_INFO, "load: " << p);
                
                // shapefile contains multiple polygons.
                // read an array of them
                polys.clear();
                tgShapefile::ToPolygons( p, polys );
                SG_LOG(SG_GENERAL, SG_INFO, "tgShapefile::ToPolygons returned " << polys.size() << " polygons" );
                
                for ( unsigned int i=0; i<polys.size(); i++ ) {
                    int area = area_defs.get_area_priority( polys[i].GetMaterial() );
                    if ( area != 0xFFFF ) {
                        std::string material = area_defs.get_area_name( area );
                    
                        //if ( material.find("Road") != std::string::npos ) {
                        //    material = "lftest";
                        //}
                    
#if 0                        // TODO
                        // add the nodes
                        for (unsigned int j=0; j<polys[i].Contours(); j++) {
                            for (unsigned int k=0; k<polys[i].ContourSize(j); k++) {
                                SGGeod node  = polys[i].GetNode( j, k );
                            
                                start.stamp();
                                if ( polys[i].GetPreserve3D() ) {
                                    nodes.unique_add( node, TG_NODE_FIXED_ELEVATION );
                                } else {
                                    nodes.unique_add( node );
                                }
                                polys[i].SetNode( j, k, node );
                                end.stamp();
                                addnode += (end-start);
                            }
                        }
#endif

                        //polys[i] = tgPolygon::RemoveCycles( poly );
                        polys[i].SetId( cur_poly_id++ );
                        
                        tgShapefile::FromPolygon( polys[i], true, false, "./loaded", material, "loaded" );

                        polys_in.add_poly( area, polys[i] );
                        total_polys_read++;
                    }
                }
            }
        }
    } else {
        SG_LOG(SG_GENERAL, SG_INFO, "directory not found: " << poly_path);        
    }
    
    SG_LOG(SG_GENERAL, SG_DEBUG, " Total polys read in this tile: " <<  total_polys_read );
    
    return total_polys_read;
}
#endif

#if 0
// load all 2d polygons from the specified load disk directories and
// clip against each other to resolve any overlaps
int TGConstruct::LoadLandclassPolys( void ) {
    int i;

    string base = bucket.gen_base_path();
    string poly_path;
    unsigned int total_polys_read = 0;
    unsigned int total_nodes = 0;
    tgPolygon poly;
    
    SGTimeStamp addnode;
    SGTimeStamp start, end;

    // load 2D polygons from all directories provided
    for ( i = 0; i < (int)load_dirs.size(); ++i ) {
        poly_path = work_base + "/" + load_dirs[i] + '/' + base;

        string tile_str = bucket.gen_index_str();
        simgear::Dir d(poly_path);
        if (!d.exists()) {
            SG_LOG(SG_GENERAL, SG_DEBUG, "directory not found: " << poly_path);
            continue;
        }

        simgear::PathList files = d.children(simgear::Dir::TYPE_FILE);
        SG_LOG( SG_CLIPPER, SG_DEBUG, files.size() << " Polys in " << d.path() );

        BOOST_FOREACH(const SGPath& p, files) {
            if (p.file_base() != tile_str) {
                continue;
            }

            string lext = p.complete_lower_extension();
            if ((lext == "arr") || (lext == "arr.gz") || (lext == "btg.gz") ||
                (lext == "fit") || (lext == "fit.gz") || (lext == "ind"))
            {
                // skipped!
            } else {
                int area;
                std::string material;
                gzFile fp = gzopen( p.c_str(), "rb" );
                unsigned int count;

                sgReadUInt( fp, &count );
                SG_LOG( SG_GENERAL, SG_INFO, " Load " << count << " polys from " << p.realpath() );

                for ( unsigned int i=0; i<count; i++ ) {
                    poly.LoadFromGzFile( fp );
                    
                    area     = area_defs.get_area_priority( poly.GetFlag() );
                    material = area_defs.get_area_name( area );

                    poly.SetMaterial( material );
                    poly.SetId( cur_poly_id++ );

                    polys_in.add_poly( area, poly );
                    total_polys_read++;

                    // add the nodes
                    for (unsigned int j=0; j<poly.Contours(); j++) {
                        for (unsigned int k=0; k<poly.ContourSize(j); k++) {
                            SGGeod node = poly.GetNode( j, k );

                            start.stamp();
                            total_nodes++;
                            //if ( poly.GetPreserve3D() ) {
                            //    nodes.unique_add( node, TG_NODE_FIXED_ELEVATION );
                            //} else {
                                nodes.unique_add( node );
                            //}
                            end.stamp();
                            addnode += (end-start);
                        }
                    }

//                    if (IsDebugShape( poly.GetId() )) {
                    if ( true ) {
                        char desc[32];
                        sprintf(desc, "loaded_%d", poly.GetId() );

                        tgShapefile::FromPolygon( poly, true, false, "./loaded", material, desc );
                    }
                }

                gzclose( fp );
                SG_LOG(SG_GENERAL, SG_DEBUG, " Loaded " << p.file());
            }
        } // of directory file children
    }

    SG_LOG(SG_GENERAL, SG_DEBUG, " Total polys read in this tile: " <<  total_polys_read );
    
    SG_LOG( SG_GENERAL, SG_INFO, " added " << total_nodes << " unique nodes in " << addnode );
    
    
    return total_polys_read;
}
#endif

#if 0
bool TGConstruct::CheckMatchingNode( SGGeod& node, bool road, bool fixed )
{
    bool   matched = false;
    bool   added = false;
    
    if ( fixed ) {
        nodes.unique_add( node, TG_NODE_FIXED_ELEVATION );
        // node = nodes.unique_add_fixed_elevation( node );
        added = true;
    } else {
        // check mutable edge
        if ( !nm_north.empty() ) {
            double north_compare = bucket.get_center_lat() + 0.5 * bucket.get_height();
            if ( fabs(node.getLatitudeDeg() - north_compare) < SG_EPSILON) {
                if ( !road ) {
                    // node is on the non_mutable northern border - get closest pt
                    node = GetNearestNodeLongitude( node, nm_north );
                    SG_LOG(SG_GENERAL, SG_DEBUG, " AddNode: constrained on north border from " <<  node << " to " << node );
                    added = true;
                } else {
                    matched = true;
                }
            } 
        } 
        
        if ( !added && !matched && !nm_south.empty() ) {
            double south_compare = bucket.get_center_lat() - 0.5 * bucket.get_height();
            if ( fabs(node.getLatitudeDeg() - south_compare) < SG_EPSILON) {
                if ( !road ) {
                    // node is on the non_mutable southern border - get closest pt
                    node = GetNearestNodeLongitude( node, nm_south );
                    SG_LOG(SG_GENERAL, SG_DEBUG, " AddNode: constrained on south border from " <<  node << " to " << node );
                    added = true;
                } else {
                    matched = true;
                }
            }
        } 
        
        if ( !added && !matched && !nm_east.empty() ) {
            double east_compare  = bucket.get_center_lon() + 0.5 * bucket.get_width();
            if ( fabs(node.getLongitudeDeg() - east_compare) < SG_EPSILON) {
                if ( !road ) {
                    // node is on the non_mutable eastern border - get closest pt
                    node = GetNearestNodeLatitude( node, nm_east );
                    SG_LOG(SG_GENERAL, SG_DEBUG, " AddNode: constrained on east border from " <<  node << " to " << node );
                    added = true;
                } else {
                    matched = true;
                }
            }
        } 
        
        if ( !added && !matched && !nm_west.empty() ) {
            double west_compare  = bucket.get_center_lon() - 0.5 * bucket.get_width();
            if ( fabs(node.getLongitudeDeg() - west_compare) < SG_EPSILON) {
                if ( !road ) {
                    // node is on the non_mutable western border - get closest pt
                    node = GetNearestNodeLatitude( node, nm_west );
                    SG_LOG(SG_GENERAL, SG_DEBUG, " AddNode: constrained on west border from " <<  node << " to " << node );                
                    added = true;
                } else {
                    matched = true;
                }
            }
        } 
    }
    
    if (!added && !matched) {
        nodes.unique_add( node );
        added = true;
    }
    
    return added;
}
#endif

#if 0
SGGeod TGConstruct::GetNearestNodeLongitude( const SGGeod& node, const std::vector<SGGeod>& selection )
{
    double       min_dist = std::numeric_limits<double>::infinity();
    double       cur_dist;
    unsigned int min_idx = 0;
    
    for ( unsigned int i=0; i<selection.size(); i++ ) {
        cur_dist = fabs( node.getLongitudeDeg() - selection[i].getLongitudeDeg() );
        if ( cur_dist < min_dist ) {
            min_dist = cur_dist;
            min_idx = i;
        }
    }
    
    return selection[min_idx];
}
#endif

#if 0
SGGeod TGConstruct::GetNearestNodeLatitude( const SGGeod& node, const std::vector<SGGeod>& selection )
{
    double       min_dist = std::numeric_limits<double>::infinity();
    double       cur_dist;
    unsigned int min_idx = 0;
    
    for ( unsigned int i=0; i<selection.size(); i++ ) {
        cur_dist = fabs( node.getLatitudeDeg() - selection[i].getLatitudeDeg() );
        if ( cur_dist < min_dist ) {
            min_dist = cur_dist;
            min_idx = i;
        }
    }
    
    return selection[min_idx];
}
#endif