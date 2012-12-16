#include <simgear/debug/logstream.hxx>

#include "tg_accumulator.hxx"
#include "tg_shapefile.hxx"
#include "tg_misc.hxx"

tgPolygon tgAccumulator::Diff( const tgContour& subject )
{
    tgPolygon  result;
    UniqueSGGeodSet all_nodes;

    /* before diff - gather all nodes */
    for ( unsigned int i = 0; i < subject.GetSize(); ++i ) {
        all_nodes.add( subject.GetNode(i) );
    }

    unsigned int  num_hits = 0;
    tgRectangle box1 = subject.GetBoundingBox();

    ClipperLib::Polygon  clipper_subject = tgContour::ToClipper( subject );
    ClipperLib::Polygons clipper_result;

    ClipperLib::Clipper c;
    c.Clear();

    c.AddPolygon(clipper_subject, ClipperLib::ptSubject);

    // clip result against all polygons in the accum that intersect our bb
    for (unsigned int i=0; i < accum.size(); i++) {
        tgRectangle box2 = BoundingBox_FromClipper( accum[i] );

        if ( box2.intersects(box1) )
        {
            c.AddPolygons(accum[i], ClipperLib::ptClip);
            num_hits++;
        }
    }

    if (num_hits) {
        if ( !c.Execute(ClipperLib::ctDifference, clipper_result, ClipperLib::pftNonZero, ClipperLib::pftNonZero) ) {
            SG_LOG(SG_GENERAL, SG_ALERT, "Diff With Accumulator returned FALSE" );
            exit(-1);
        }
        result = tgPolygon::FromClipper( clipper_result );
        result = tgPolygon::AddColinearNodes( result, all_nodes );
    } else {
        result.AddContour( subject );
    }

    return result;
}

void tgAccumulator::Add( const tgContour& subject )
{
    tgPolygon poly;
    poly.AddContour( subject );

    ClipperLib::Polygons clipper_subject = tgPolygon::ToClipper( poly );
    accum.push_back( clipper_subject );
}

void tgAccumulator::ToShapefiles( const std::string& path, const std::string& layer_prefix )
{
    char shapefile[16];
    char layer[16];

    for (unsigned int i=0; i < accum.size(); i++) {
        sprintf( layer, "%s_%d", layer_prefix.c_str(), i );
        sprintf( shapefile, "accum_%d", i );
        tgShapefile::FromClipper( accum[i], path, layer, std::string(shapefile) );
    }
}

tgPolygon tgAccumulator::Diff( const tgPolygon& subject )
{
    tgPolygon result;
    UniqueSGGeodSet all_nodes;

    /* before diff - gather all nodes */
    for ( unsigned int i = 0; i < subject.Contours(); ++i ) {
        for ( unsigned int j = 0; j < subject.ContourSize( i ); ++j ) {
            all_nodes.add( subject.GetNode(i, j) );
        }
    }

    unsigned int  num_hits = 0;
    tgRectangle box1 = subject.GetBoundingBox();

    ClipperLib::Polygons clipper_subject = tgPolygon::ToClipper( subject );
    ClipperLib::Polygons clipper_result;

    ClipperLib::Clipper c;
    c.Clear();

    c.AddPolygons(clipper_subject, ClipperLib::ptSubject);

    // clip result against all polygons in the accum that intersect our bb
    for (unsigned int i=0; i < accum.size(); i++) {
        tgRectangle box2 = BoundingBox_FromClipper( accum[i] );

        if ( box2.intersects(box1) )
        {
            c.AddPolygons(accum[i], ClipperLib::ptClip);
            num_hits++;
        }
    }

    if (num_hits) {
        if ( !c.Execute(ClipperLib::ctDifference, clipper_result, ClipperLib::pftNonZero, ClipperLib::pftNonZero) ) {
            SG_LOG(SG_GENERAL, SG_ALERT, "Diff With Accumulator returned FALSE" );
            exit(-1);
        }

        result = tgPolygon::FromClipper( clipper_result );
        result = tgPolygon::AddColinearNodes( result, all_nodes );

        // Make sure we keep texturing info
        result.SetMaterial( subject.GetMaterial() );
        result.SetTexParams( subject.GetTexParams() );
    } else {
        result = subject;
    }

    return result;
}

void tgAccumulator::Add( const tgPolygon& subject )
{
    ClipperLib::Polygons clipper_subject = tgPolygon::ToClipper( subject );
    accum.push_back( clipper_subject );
}
