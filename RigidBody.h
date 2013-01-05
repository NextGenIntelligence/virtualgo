#ifndef RIGID_BODY_H
#define RIGID_BODY_H

/*
    Rigid body class and support functions.
    We need a nice way to cache the local -> world,
    world -> local and position for a given rigid body.

    The rigid body transform class lets us do this,
    it's fundamentally a 4x4 rigid body transform matrix
    but with the inverse cached, as well as position, rotation
    and rotation inverse for quick lookup.
*/

void CalculateSphereInertiaTensor( float mass, float r, mat4f & inertiaTensor, mat4f & inverseInertiaTensor )
{
    const float i = 2.0f / 5.0f * mass * r * r;
    float values[] = { i, 0, 0, 0, 
                       0, i, 0, 0,
                       0, 0, i, 0,
                       0, 0, 0, 1 };
    float inverse_values[] = { 1/i,  0,   0,   0, 
                                0, 1/i,   0,   0, 
                                0,   0, 1/i,   0,
                                0,   0,   0,   1 };
    inertiaTensor.load( values );
    inverseInertiaTensor.load( inverse_values );
}

void CalculateEllipsoidInertiaTensor( float mass, float a, float b, float c, mat4f & inertiaTensor, mat4f & inverseInertiaTensor )
{
    const float i_a = 1.0f/5.0f * mass * ( b*b + c*c );
    const float i_b = 1.0f/5.0f * mass * ( a*a + c*c );
    const float i_c = 1.0f/5.0f * mass * ( a*a + b*b );
    float values[] = { i_a,   0,   0, 0, 
                         0, i_b,   0, 0,
                         0,   0, i_c, 0,
                         0,   0,   0, 1 };
    float inverse_values[] = { 1/i_a,     0,     0,   0, 
                                   0, 1/i_b,     0,   0, 
                                   0,     0, 1/i_c,   0,
                                   0,     0,     0,   1 };
    inertiaTensor.load( values );
    inverseInertiaTensor.load( inverse_values );
}

float CalculateBiconvexVolume( const Biconvex & biconvex )
{
    const float r = biconvex.GetSphereRadius();
    const float h = r - biconvex.GetHeight() / 2;
    return h*h + ( pi * r / 4 + pi * h / 24 );
}

void CalculateBiconvexInertiaTensor( float mass, const Biconvex & biconvex, mat4f & inertiaTensor, mat4f & inverseInertiaTensor )
{
    const double resolution = 0.01;
    const double width = biconvex.GetWidth();
    const double height = biconvex.GetHeight();
    const double xz_steps = ceil( width / resolution );
    const double y_steps = ceil( height / resolution );
    const double dx = width / xz_steps;
    const double dy = height / y_steps;
    const double dz = width / xz_steps;
    double sx = -width / 2;
    double sy = -height / 2;
    double sz = -width / 2;
    double ix = 0.0;
    double iy = 0.0;
    double iz = 0.0;
    const double v = CalculateBiconvexVolume( biconvex );
    const double p = mass / v;
    const double m = dx*dy*dz * p;
    for ( int index_z = 0; index_z <= xz_steps; ++index_z )
    {
        for ( int index_y = 0; index_y <= y_steps; ++index_y )
        {
            for ( int index_x = 0; index_x <= xz_steps; ++index_x )
            {
                const double x = sx + index_x * dx;
                const double y = sy + index_y * dy;
                const double z = sz + index_z * dz;

                vec3f point(x,y,z);

                if ( PointInsideBiconvex_LocalSpace( point, biconvex ) )
                {
                    const double rx2 = z*z + y*y;
                    const double ry2 = x*x + z*z;
                    const double rz2 = x*x + y*y;

                    ix += rx2 * m;
                    iy += ry2 * m;
                    iz += rz2 * m;
                }
            }
        }
    }

    {
        // http://wolframalpha.com
        // integrate ( r^2 - ( y + r - h/2 ) ^ 2 ) ^ 2 dy from y = 0 to h/2
        //  => 1/480 h^3 (3 h^2-30 h r+80 r^2)
        const float h = height;
        const float r = biconvex.GetSphereRadius();
        const float h2 = h * h;
        const float h3 = h2 * h;
        const float h4 = h3 * h;
        const float h5 = h4 * h;
        const float r2 = r * r;
        const float r3 = r2 * r;
        const float r4 = r3 * r;
        const float exact_iy = pi * p * ( 1/480.0f * h3 * ( 3*h2 - 30*h*r + 80*r2 ) );
    }

    const float values[] = { ix,  0,  0, 0, 
                              0, iy,  0, 0, 
                              0,  0, iz, 0, 
                              0,  0,  0, 1 };
    const float inverseValues[] = { 1/ix,    0,    0, 0, 
                                       0, 1/iy,    0, 0, 
                                       0,    0, 1/iz, 0, 
                                       0,    0,    0, 1 };
    inertiaTensor.load( values );
    inverseInertiaTensor.load( inverseValues );
}

struct RigidBody
{
    vec3f position;
    quat4f orientation;
    vec3f linearVelocity;
    vec3f angularVelocity;
    float mass;
    float inverseMass;
    mat4f inertiaTensor;
    mat4f inverseInertiaTensor;

    RigidBody()
    {
        position = vec3f(0,0,0);
        orientation = quat4f::identity();
        linearVelocity = vec3f(0,0,0);
        angularVelocity = vec3f(0,0,0);
        mass = 1.0f;
        inverseMass = 1.0f / mass;
        inertiaTensor = mat4f::identity();
        inverseInertiaTensor = mat4f::identity();
    }

    vec3f GetVelocityAtPoint( vec3f point ) const
    {
        return linearVelocity + cross( angularVelocity, point - position );
    }

    float GetKineticEnergy() const
    {
        const float linearKE = 0.5f * mass * length_squared( linearVelocity );
        const float angularKE = 0.5f * dot( transformVector( inertiaTensor, angularVelocity ), angularVelocity );
        return linearKE + angularKE;
    }
};

inline quat4f AngularVelocityToSpin( const quat4f & orientation, vec3f angularVelocity )
{
    return 0.5f * quat4f( 0, angularVelocity.x(), angularVelocity.y(), angularVelocity.z() ) * orientation;
}

inline mat4f RigidBodyInverse( const mat4f & matrix )
{
    /*
        How to invert a rigid body matrix
        http://graphics.stanford.edu/courses/cs248-98-fall/Final/q4.html
    */

    mat4f inverse = matrix;
    
    vec4f translation = matrix.value.w;

    inverse.value.w = simd4f_create(0,0,0,1);
    simd4x4f_transpose_inplace( &inverse.value );

    inverse.value.w = simd4f_create( -dot( matrix.value.x, translation ),
                                     -dot( matrix.value.y, translation ),
                                     -dot( matrix.value.z, translation ),
                                     1.0f );

    return inverse;
}

struct RigidBodyTransform
{
    mat4f localToWorld;
    mat4f worldToLocal;

    RigidBodyTransform( vec3f position, mat4f rotation = mat4f::identity() )
    {
        localToWorld = rotation;
        localToWorld.value.w = simd4f_create( position.x(), position.y(), position.z(), 1 );
        worldToLocal = RigidBodyInverse( localToWorld );
    }

    RigidBodyTransform( vec3f position, quat4f rotation )
    {
        rotation.toMatrix( localToWorld );
        localToWorld.value.w = simd4f_create( position.x(), position.y(), position.z(), 1 );
        worldToLocal = RigidBodyInverse( localToWorld );
    }

    vec3f GetPosition() const
    {
        return localToWorld.value.w;
    }

    vec3f GetUp() const
    {
        return transformVector( localToWorld, vec3f(0,1,0) );
    }
};

#endif