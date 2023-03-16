#pragma once


namespace Hk::Math
{
	/**
	 * Computes the difference between two points in 3d space,
	 * @param v1 3d vector 1
	 * @param v2 3d vector 2
	 * @returns a scalar of the distance between point v2 and v1
	 */
	DLL float Distance3D(Vector v1, Vector v2);

	/**
	 * See Distance3D for more information
	 * @see Distance3D
	 */
	DLL cpp::result<float, Error> Distance3DByShip(uint ship1, uint ship2);

	/**
	 * Converts a 3x3 rotation matrix into an equivalent quaternion.
	 */
	DLL Quaternion MatrixToQuaternion(const Matrix& m);
	template<typename Str>
	Str VectorToSectorCoord(uint systemId, Vector pos);
	template DLL std::string VectorToSectorCoord(uint systemId, Vector pos);
	template DLL std::wstring VectorToSectorCoord(uint systemId, Vector pos);
	DLL float Degrees(float rad);
	DLL Vector MatrixToEuler(const Matrix& mat);
	DLL uint RgbToBgr(uint color);
	DLL std::wstring UintToHexString(uint number, uint width, bool addPrefix = false);
}