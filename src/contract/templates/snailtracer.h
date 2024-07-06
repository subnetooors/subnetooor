/*
Copyright (c) [2023-2024] [AppLayer Developers]

This software is distributed under the MIT License.
See the LICENSE.txt file in the project root for more information.
*/

#ifndef SNAILTRACER_H
#define SNAILTRACER_H

#include <tuple>

#include "../dynamiccontract.h"
#include "../variables/safebytes.h"
#include "../variables/safeint.h"
#include "../variables/safetuple.h"
#include "../variables/safeuint.h"
#include "../variables/safevector.h"
#include "../../utils/utils.h" // int/uint/bytes aliases

/**
 * Ray tracer contract that creates a scene and pre-calculates some constants
 * that are the same throughout the path tracing procedure.
 */
class SnailTracer : public DynamicContract {
  private:
    /// Enum for light-altering surface types.
    enum Material { Diffuse, Specular, Refractive };

    /// Enum for primitive geometric types.
    enum Primitive { PSphere, PTriangle };

    /// Struct for a 3-axis vector (X, Y, Z).
    using Vector = std::tuple<int256_t, int256_t, int256_t>;

    /// Struct for a parametric line (origin, direction, depth, refract).
    using Ray = std::tuple<Vector, Vector, int256_t, bool>;

    /// Struct for a physical sphere to intersect light rays with (radius, position, emission, color, reflection).
    using Sphere = std::tuple<int256_t, Vector, Vector, Vector, Material>;

    /// Struct for a physical triangle to intersect light rays with (a, b, c, normal, emission, color, reflection).
    using Triangle = std::tuple<Vector, Vector, Vector, Vector, Vector, Vector, Material>;

    using SVector = SafeTuple<int256_t, int256_t, int256_t>; ///< SafeVar version of Vector.
    using SRay = SafeTuple<Vector, Vector, int256_t, bool>; ///< SafeVar version of Ray.
    using SSphereArr = SafeVector<Sphere>; ///< SafeVar version of an array of Spheres.
    using STriangleArr = SafeVector<Triangle>; ///< SafeVar version of an array of Triangles.

    SafeInt256_t width_; ///< Width of the image to be generated (fixed for life).
    SafeInt256_t height_; ///< Height of the image to be generated (fixed for life).
    SafeBytes buffer_; ///< Ephemeral buffer to accumulate image traces.
    SafeUint32_t seed_;  ///< Trivial linear congruential pseudo-random seed generated by rand().
    SRay camera_;  ///< Camera position for image assembly.
    SVector deltaX_; ///< Horizontal FoV angle increment per image pixel.
    SVector deltaY_; ///< Vertical FoV angle increment per image pixel.
    SSphereArr spheres_; ///< Array of spheres defining the scene to render.
    STriangleArr triangles_; ///< Array of triangles defining the scene to render.

    void registerContractFunctions() override; ///< Register the contract functions.

  public:
    /// The constructor argument types.
    using ConstructorArguments = std::tuple<int256_t, int256_t>;

    /**
     * Constructor to be used when creating a new contract.
     * @param w Width to initialize with.
     * @param h Height to initialize with.
     * @param address The address where the contract will be deployed.
     * @param creator The address of the creator of the contract.
     * @param chainId The chain where the contract wil be deployed.
     */
    SnailTracer(
      int256_t w, int256_t h, const Address& address, const Address& creator, const uint64_t& chainId
    );

    /**
     * Constructor for loading contract from DB.
     * @param address The address where the contract will be deployed.
     * @param db Reference to the database object.
     */
    SnailTracer(const Address& address, const DB& db);

    ~SnailTracer() override; ///< Destructor.

    /**
     * Trace a single pixel of the configured image.
     * Meant to be used specifically for high SPP renderings which would have a huge overhead otherwise.
     * @param x The X position of the pixel to trace.
     * @param y The Y position of the pixel to trace.
     * @param spp The SPP (Samples Per Pixel) of the pixel to trace.
     * @return The R, G and B values respectively.
     */
    std::tuple<uint8_t, uint8_t, uint8_t> TracePixel(const int256_t& x, const int256_t& y, const uint256_t& spp);

    /**
     * Trace a single horizontal scanline of the configured image.
     * Used for lower SPP rendering to avoid overhead of by-pixel calls.
     * @param y The Y position of the line to trace.
     * @param spp The SPP (Samples Per Pixel) of the line to trace.
     * @return The RGB value pixel array.
     */
    Bytes TraceScanline(const int256_t& y, const int256_t& spp);

    /**
     * Trace an entire image of the configured scene.
     * Used only for very small images and SPP values to cut down on cumulative gas and memory costs.
     * @param spp The SPP (Samples Per Pixel) of the image to trace.
     * @return The RGB pixel value array, top-down, left-to-right.
     */
    Bytes TraceImage(const int256_t& spp);

    /**
     * Set up an ephemeral image configuration and trace a select few
     * hand picked pixels from it to measure execution performance.
     * @return The R, G and B values respectively.
     */
    std::tuple<uint8_t, uint8_t, uint8_t> Benchmark();

    /**
     * Execute path tracing for a single pixel of the result image.
     * @param x The X position of the pixel to trace.
     * @param y The Y position of the pixel to trace.
     * @param spp The SPP (Samples Per Pixel) of the pixel to trace.
     * @return The RGB color vector normalized to [0, 256) value range.
     */
    Vector trace(const int256_t& x, const int256_t& y, const int256_t& spp);

    uint32_t rand(); ///< Trivial linear congruential pseudo-random number generator. Returns a new random seed.

    /**
     * Bound an int value to the allowed [0, 1] range.
     * @param x The value to clamp.
     * @return The clamped value.
     */
    int256_t clamp(const int256_t& x);

    /**
     * Calculate the square root of an int value based on the Babylonian method.
     * @param x The value to calculate the square root from.
     * @return The square root of the value.
     */
    int256_t sqrt(const int256_t& x);

    /**
     * Calculate the sine of an int value based on Taylor series expansion.
     * @param x The value to calculate the sine of.
     * @return The sine of the value.
     */
    int256_t sin(int256_t x);

    /**
     * Calculate the cosine of an int value based on sine and Pythagorean identity.
     * @param The value to calculate the cosine of.
     * @return The cosine of the value.
     */
    int256_t cos(const int256_t& x);

    /**
     * Get the absolute value of an int.
     * @param x The value to get the absolute value of.
     * @return The absolute value of the value.
     */
    int256_t abs(const int256_t& x);

    /**
     * Add the internal values of two vectors.
     * @param u The first vector to add from.
     * @param v The second vector to add from.
     * @return A new vector with the resulting sum of both vectors.
     */
    Vector add(const Vector& u, const Vector& v);

    /**
     * Subtract the internal values of two vectors.
     * @param u The first vector to subtract from.
     * @param v The second vector to subtract from.
     * @return A new vector with the resulting subtraction of both vectors.
     */
    Vector sub(const Vector& u, const Vector& v);

    /**
     * Multiply the internal values of two vectors.
     * @param u The first vector to multiply from.
     * @param v The second vector to multiply from.
     * @return A new vector with the resulting multiplication of both vectors.
     */
    Vector mul(const Vector& u, const Vector& v);

    /**
     * Overload of mul() that takes an integer instead of a second vector.
     * @param v The vector to multiply from.
     * @param m The integer to multiply from.
     * @return A new vector with the resulting multiplication of the vector by the integer.
     */
    Vector mul(const Vector& v, const int256_t& m);

    /**
     * Divide the internal values of a vector by a given integer.
     * @param v The vector to divide from.
     * @param d The integer to divide from.
     * @return A new vector with the resulting division of the vector by the integer.
     */
    Vector div(const Vector& v, const int256_t& d);

    /**
     * Calculate the dot product between two vectors.
     * @param u The first vector to get the dot product from.
     * @param v The second vector to get the dot product from.
     * @return The dot product value as an integer.
     */
    int256_t dot(const Vector& u, const Vector& v);

    /**
     * Calculate the cross product between two vectors.
     * @param u The first vector to get the cross product from.
     * @param v The second vector to get the cross product from.
     * @return A new vector with the resulting cross product from both vectors.
     */
    Vector cross(const Vector& u, const Vector& v);

    /**
     * Calculate the normalization for a given vector.
     * @param v The vector to normalize.
     * @return A normalized copy of the vector.
     */
    Vector norm(const Vector& v);

    /**
     * Bound a vector's values to the allowed [0, 1] range.
     * @param v The vector to clamp.
     * @return A clamped copy of the vector.
     */
    Vector clamp(const Vector& v);

    /**
     * Calculate the intersection of a ray with a sphere.
     * @param s The sphere to intersect with.
     * @param r The ray to intersect with.
     * @return The distance until the first intersection point, or zero in case of no intersection.
     */
    int256_t intersect(const Sphere& s, const Ray& r);

    /**
     * Calculate the intersection of a ray with a triangle.
     * @param t The triangle to intersect with.
     * @param r The ray to intersect with.
     * @return The distance until the first intersection point, or zero in case of no intersection.
     */
    int256_t intersect(const Triangle& t, const Ray& r);

    /**
     * Calculate the radiance of a ray.
     * @param ray The ray to calculate the radiance from.
     * @return A vector representing the radiance.
     */
    Vector radiance(Ray& ray);

    /**
     * Overload of radiance() for spheres.
     * @param ray The ray to calculate the radiance from.
     * @param obj The Sphere object to use as base for calculation.
     * @param dist The distance between the ray and the object.
     * @return A vector representing the radiance.
     */
    Vector radiance(const Ray& ray, const Sphere& obj, const int256_t& dist);

    /**
     * Overload of radiance() for triangles.
     * @param ray The ray to calculate the radiance from.
     * @param obj The Triangle object to use as base for calculation.
     * @param dist The distance between the ray and the object.
     * @return A vector representing the radiance.
     */
    Vector radiance(const Ray& ray, const Triangle& obj, const int256_t& dist);

    /**
     * Calculate the diffusion of a ray.
     * @param ray The ray to calculate the diffusion from.
     * @param intersect A vector representing an intersection.
     * @param normal A vector representing the normal.
     * @return A vector representing the diffusion.
     */
    Vector diffuse(const Ray& ray, const Vector& intersect, const Vector& normal);

    /**
     * Calculate the specular reflection of a ray.
     * @param ray The ray to calculate the specular from.
     * @param intersect A vector representing an intersection.
     * @param normal A vector representing the normal.
     * @return A vector representing the specular.
     */
    Vector specular(const Ray& ray, const Vector& intersect, const Vector& normal);

    /**
     * Calculate the refractive reflection of a ray.
     * @param ray The ray to calculate the refractive from.
     * @param intersect A vector representing an intersection.
     * @param normal A vector representing the normal.
     * @param nnt TODO
     * @param ddn TODO
     * @param cos2t TODO
     * @return A vector representing the refractive.
     */
    Vector refractive(
      const Ray& ray, const Vector& intersect, const Vector& normal,
      const int256_t& nnt, const int256_t& ddn, const int256_t& cos2t
    );

    /**
     * Calculate the intersection of a ray with all the objects.
     * @param ray The ray to intersect.
     * @return The intersection of the ray with the closest object.
     */
    std::tuple<int256_t, Primitive, uint256_t> traceray(const Ray& ray);

    /// Register the contract structure.
    static void registerContract() {
      ContractReflectionInterface::registerContractMethods<
        SnailTracer, int256_t&, int256_t&,
        const Address&, const Address&, const uint64_t&, DB&
      >(
        std::vector<std::string>{"width_", "height_", "buffer_", "seed_", "camera_", "deltaX_", "deltaY_", "spheres_", "triangles_"},
        std::make_tuple("TracePixel", &SnailTracer::TracePixel, FunctionTypes::NonPayable, std::vector<std::string>{"x", "y", "spp"}),
        std::make_tuple("TraceScanline", &SnailTracer::TraceScanline, FunctionTypes::NonPayable, std::vector<std::string>{"y", "spp"}),
        std::make_tuple("TraceImage", &SnailTracer::TraceImage, FunctionTypes::NonPayable, std::vector<std::string>{"spp"}),
        std::make_tuple("Benchmark", &SnailTracer::Benchmark, FunctionTypes::NonPayable, std::vector<std::string>{}),
        std::make_tuple("trace", &SnailTracer::trace, FunctionTypes::NonPayable, std::vector<std::string>{"x", "y", "spp"}),
        std::make_tuple("rand", &SnailTracer::rand, FunctionTypes::NonPayable, std::vector<std::string>{}),
        std::make_tuple("clamp", static_cast<int256_t(SnailTracer::*)(const int256_t&)>(&SnailTracer::clamp), FunctionTypes::NonPayable, std::vector<std::string>{"x"}),
        std::make_tuple("sqrt", &SnailTracer::sqrt, FunctionTypes::NonPayable, std::vector<std::string>{"x"}),
        std::make_tuple("sin", &SnailTracer::sin, FunctionTypes::NonPayable, std::vector<std::string>{"x"}),
        std::make_tuple("cos", &SnailTracer::cos, FunctionTypes::NonPayable, std::vector<std::string>{"x"}),
        std::make_tuple("abs", &SnailTracer::abs, FunctionTypes::NonPayable, std::vector<std::string>{"x"}),
        std::make_tuple("add", &SnailTracer::add, FunctionTypes::NonPayable, std::vector<std::string>{"u", "v"}),
        std::make_tuple("sub", &SnailTracer::sub, FunctionTypes::NonPayable, std::vector<std::string>{"u", "v"}),
        std::make_tuple("mul", static_cast<Vector(SnailTracer::*)(const Vector&,const Vector&)>(&SnailTracer::mul), FunctionTypes::NonPayable, std::vector<std::string>{"u", "v"}),
        std::make_tuple("mul", static_cast<Vector(SnailTracer::*)(const Vector&,const int256_t&)>(&SnailTracer::mul), FunctionTypes::NonPayable, std::vector<std::string>{"v", "m"}),
        std::make_tuple("div", &SnailTracer::div, FunctionTypes::NonPayable, std::vector<std::string>{"v", "d"}),
        std::make_tuple("dot", &SnailTracer::dot, FunctionTypes::NonPayable, std::vector<std::string>{"u", "v"}),
        std::make_tuple("cross", &SnailTracer::cross, FunctionTypes::NonPayable, std::vector<std::string>{"u", "v"}),
        std::make_tuple("norm", &SnailTracer::norm, FunctionTypes::NonPayable, std::vector<std::string>{"v"}),
        std::make_tuple("clamp", static_cast<Vector(SnailTracer::*)(const Vector&)>(&SnailTracer::clamp), FunctionTypes::NonPayable, std::vector<std::string>{"v"}),
        std::make_tuple("intersect", static_cast<int256_t(SnailTracer::*)(const Sphere&, const Ray&)>(&SnailTracer::intersect), FunctionTypes::NonPayable, std::vector<std::string>{"s", "r"}),
        std::make_tuple("intersect", static_cast<int256_t(SnailTracer::*)(const Triangle&, const Ray&)>(&SnailTracer::intersect), FunctionTypes::NonPayable, std::vector<std::string>{"t", "r"}),
        std::make_tuple("radiance", static_cast<Vector(SnailTracer::*)(Ray&)>(&SnailTracer::radiance), FunctionTypes::NonPayable, std::vector<std::string>{"ray"}),
        std::make_tuple("radiance", static_cast<Vector(SnailTracer::*)(const Ray&, const Sphere&, const int256_t&)>(&SnailTracer::radiance), FunctionTypes::NonPayable, std::vector<std::string>{"ray", "obj", "dist"}),
        std::make_tuple("radiance", static_cast<Vector(SnailTracer::*)(const Ray&, const Triangle&, const int256_t&)>(&SnailTracer::radiance), FunctionTypes::NonPayable, std::vector<std::string>{"ray", "obj", "dist"}),
        std::make_tuple("diffuse", &SnailTracer::diffuse, FunctionTypes::NonPayable, std::vector<std::string>{"ray", "intersect", "normal"}),
        std::make_tuple("specular", &SnailTracer::specular, FunctionTypes::NonPayable, std::vector<std::string>{"ray", "intersect", "normal"}),
        std::make_tuple("refractive", &SnailTracer::refractive, FunctionTypes::NonPayable, std::vector<std::string>{"ray", "intersect", "normal", "nnt", "ddn", "cos2t"}),
        std::make_tuple("traceray", &SnailTracer::traceray, FunctionTypes::NonPayable, std::vector<std::string>{"ray"})
      );
    }

    DBBatch dump() const override; ///< Dump method.
};

#endif // SNAILTRACER_H
