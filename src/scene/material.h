//
// material.h
//
// The Material class: a description of the phsyical properties of a surface
// that are used to determine how that surface interacts with light.

#ifndef __MATERIAL_H__
#define __MATERIAL_H__

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/glm.hpp>
#include <string>
#include <vector>
#include <stdint.h>

class Scene;
class ray;
class isect;

using std::string;

/* The TextureMap class can be used to store a texture map,
   which consists of a bitmap and various accessors to
   it.  To implement basic texture mapping, you'll want to 
   fill in the getMappedValue function to implement basic 
   texture mapping.
*/
class TextureMap {
    public:
       TextureMap( string filename );

       // Return the mapped value; here the coordinate
       // is assumed to be within the parametrization space:
       // [0, 1] x [0, 1]
       // (i.e., {(u, v): 0 <= u <= 1 and 0 <= v <= 1}
       glm::dvec3 getMappedValue( const glm::dvec2& coord ) const;

       // Retrieve the value stored in a physical location
       // (with integer coordinates) in the bitmap.
       // Should be called from getMappedValue in order to
       // do bilinear interpolation.
       glm::dvec3 getPixelAt( int x, int y ) const;

	   int getWidth() const { return width; }
	   int getHeight() const { return height; }

	  ~TextureMap() { }
protected:
       int width;
       int height;
       std::vector<uint8_t> data;
};

class TextureMapException {
	public:
		TextureMapException( string errorMsg ) : _errorMsg( errorMsg ) {}
		string message() { return _errorMsg; }

	private:
		string _errorMsg;
};

/*
MaterialParameter is a helper class for a material;
it stores either a constant value (in a 3-vector) 
or else a link to a map of some type.  If the pointer
to the TextureMap is NULL, the class simply returns
whatever is stored in the constant vector.

This is unabashedly a speed hack; we are replacing a 
potential use of polymorphism with an old-school "if" 
statement.  But raytracers are slow enough, and the
Material class is used often enough, that it is
(somewhat) justifiable to do this.  
*/

class MaterialParameter
{
public:
    explicit MaterialParameter( const glm::dvec3& par )
      : _value( par ), _textureMap( 0 )
    { }

    explicit MaterialParameter( const double par )
      : _value( par, par, par ), _textureMap( 0 )
    { }

    explicit MaterialParameter( TextureMap* tex )
       : _textureMap( tex )
    { }

    MaterialParameter()
       : _value( 0.0, 0.0, 0.0 ), _textureMap( 0 )
    { }

    MaterialParameter& operator*=( const MaterialParameter& rhs )
    {
      (*this) *= rhs._value;
      return *this;
    }

    glm::dvec3& operator*=( const glm::dvec3& rhs )
    {
      _value[0] *= rhs[0];
      _value[1] *= rhs[1];
      _value[2] *= rhs[2];
      return _value;
    }

    glm::dvec3& operator*=( const double rhs )
    {
      _value[0] *= rhs;
      _value[1] *= rhs;
      _value[2] *= rhs;
      return _value;
    }

    MaterialParameter& operator+=( const MaterialParameter& rhs )
    {
      _value += rhs._value;
      return *this;
    }

    void setValue( const glm::dvec3& rhs )
    {
      _value = rhs;
      _textureMap = 0;
    }

    void setValue( const double rhs )
    {
      _value[0] = rhs;
      _value[1] = rhs;
      _value[2] = rhs;
      _textureMap = 0;
    }

	bool isZero() { return glm::length(_value) == 0.0; }

    glm::dvec3& operator+=( const glm::dvec3& rhs )
    {
      _value += rhs;
      return _value;
    }

    glm::dvec3 value( const isect& is ) const;
    double intensityValue( const isect& is ) const;

	// Use this to determine if the particular parameter is
	// mapped; use this to determine if we need to somehow renormalize.
	bool mapped() const { return _textureMap != 0; }

private:
    glm::dvec3 _value;
    TextureMap* _textureMap;
};

class Material
{

public:
    Material()
        : _ke( glm::dvec3( 0.0, 0.0, 0.0 ) )
        , _ka( glm::dvec3( 0.0, 0.0, 0.0 ) )
        , _ks( glm::dvec3( 0.0, 0.0, 0.0 ) )
        , _kd( glm::dvec3( 0.0, 0.0, 0.0 ) )
        , _kr( glm::dvec3( 0.0, 0.0, 0.0 ) )
        , _kt( glm::dvec3( 0.0, 0.0, 0.0 ) )
		, _refl(0)
		, _trans(0)
        , _shininess( 0.0 ) 
		, _index(1.0) {}

    virtual ~Material();

    Material( const glm::dvec3& e, const glm::dvec3& a, const glm::dvec3& s, 
              const glm::dvec3& d, const glm::dvec3& r, const glm::dvec3& t, double sh, double in )
        : _ke( e ), _ka( a ), _ks( s ), _kd( d ), _kr( r ), _kt( t ), 
          _shininess( glm::dvec3(sh,sh,sh) ), _index( glm::dvec3(in,in,in) ) { setBools(); }

    virtual glm::dvec3 shade( Scene *scene, const ray& r, const isect& i ) const;


    
    Material &
    operator+=( const Material &m )
    {
        _ke += m._ke;
        _ka += m._ka;
        _ks += m._ks;
        _kd += m._kd;
        _kr += m._kr;
        _kt += m._kt;
        _index += m._index;
        _shininess += m._shininess;
        return *this;
    }

    friend Material operator*( double d, Material m );

    // Accessor functions; we pass in an isect& for cases where
    // the parameter is dependent on, for example, world-space
    // coordinates (i.e., solid textures) or parametrized coordinates
    // (i.e., mapped textures)
    glm::dvec3 ke( const isect& i ) const { return _ke.value(i); }
    glm::dvec3 ka( const isect& i ) const { return _ka.value(i); }
    glm::dvec3 ks( const isect& i ) const { return _ks.value(i); }
    glm::dvec3 kd( const isect& i ) const { return _kd.value(i); }
    glm::dvec3 kr( const isect& i ) const { return _kr.value(i); }
    glm::dvec3 kt( const isect& i ) const { return _kt.value(i); }
    double shininess( const isect& i ) const
	{
		// Have to renormalize into the range 0-128 if it's texture mapped.
		return _shininess.mapped() ? 
			128.0*_shininess.intensityValue(i) :
			_shininess.intensityValue(i);
	}

    double index( const isect& i ) const { return _index.intensityValue(i); }

    // setting functions accepting primitives (glm::dvec3 and double)
    void setEmissive( const glm::dvec3& ke )     { _ke.setValue( ke ); }
    void setAmbient( const glm::dvec3& ka )      { _ka.setValue( ka ); }
    void setSpecular( const glm::dvec3& ks )     { _ks.setValue( ks ); setBools(); }
    void setDiffuse( const glm::dvec3& kd )      { _kd.setValue( kd ); }
    void setReflective( const glm::dvec3& kr )   { _kr.setValue( kr ); setBools(); }
    void setTransmissive( const glm::dvec3& kt ) { _kt.setValue( kt ); setBools(); }
    void setShininess( double shininess )   
                                            { _shininess.setValue( shininess ); }
    void setIndex( double index )           { _index.setValue( index ); }


    // setting functions taking MaterialParameters
    void setEmissive( const MaterialParameter& ke )            { _ke = ke; }
    void setAmbient( const MaterialParameter& ka )             { _ka = ka; }
    void setSpecular( const MaterialParameter& ks )            { _ks = ks; }
    void setDiffuse( const MaterialParameter& kd )             { _kd = kd; }
    void setReflective( const MaterialParameter& kr )          { _kr = kr; setBools(); }
    void setTransmissive( const MaterialParameter& kt )        { _kt = kt; setBools(); }
    void setShininess( const MaterialParameter& shininess )    
                                                               { _shininess = shininess; }
    void setIndex( const MaterialParameter& index )            { _index = index; }

	// get booleans for reflection and refraction
	bool Refl() const { return _refl; }
	bool Trans() const { return _trans; }
	bool Recur() const { return _recur; }
	bool Spec() const { return _spec; }
	bool Both() const { return _both; }

private:
    MaterialParameter _ke;                    // emissive
    MaterialParameter _ka;                    // ambient
    MaterialParameter _ks;                    // specular
    MaterialParameter _kd;                    // diffuse
    MaterialParameter _kr;                    // reflective
    MaterialParameter _kt;                    // transmissive

	bool _refl;								  // specular reflector?
	bool _trans;							  // specular transmitter?
	bool _recur;							  // either one
	bool _spec;								  // any kind of specular?
	bool _both;								  // reflection and transmission
    
    MaterialParameter _shininess;
    MaterialParameter _index;                 // index of refraction

	void setBools() {
		_refl = !_kr.isZero(); _trans = !_kt.isZero(); _recur = _refl || _trans;
		_spec = _refl || !_ks.isZero();
		_both = _refl && _trans;
	}

};

// This doesn't necessarily make sense for mapped materials
inline Material
operator*( double d, Material m )
{
    m._ke *= d;
    m._ka *= d;
    m._ks *= d;
    m._kd *= d;
    m._kr *= d;
    m._kt *= d;
    m._index *= d;
    m._shininess *= d;
    return m;
}

#endif // __MATERIAL_H__
