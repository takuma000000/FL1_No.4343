#define NOMINMAX
#include "ParticleEffectSettings.h"
#include "json.hpp"
#include <algorithm>
#include <array>
#include <cmath>
#include <cctype>
#include <filesystem>
#include <fstream>
#include <stdexcept>
#ifdef _WIN32
#include <Windows.h>
#endif

namespace {
	using Json = nlohmann::json;
	void Require(bool valid, const char* message) {
		if (!valid) { throw std::runtime_error(message); }
	}
	bool InRange(float v, float lo, float hi) { return std::isfinite(v) && v >= lo && v <= hi; }
	bool VectorInRange(const Vector3& v, float lo, float hi) {
		return InRange(v.x, lo, hi) && InRange(v.y, lo, hi) && InRange(v.z, lo, hi);
	}
	Json ToJson(const Vector3& v) { return Json::array({ v.x, v.y, v.z }); }
	Json ToJson(const Vector4& v) { return Json::array({ v.x, v.y, v.z, v.w }); }
	template<size_t N> std::array<float, N> ReadVector(const Json& j) {
		Require(j.is_array() && j.size() == N, "Invalid vector length");
		for (const auto& n : j) { Require(n.is_number(), "Expected a number"); }
		return j.get<std::array<float, N>>();
	}
	Vector3 Read3(const Json& j) { const auto a = ReadVector<3>(j); return { a[0], a[1], a[2] }; }
	Vector4 Read4(const Json& j) { const auto a = ReadVector<4>(j); return { a[0], a[1], a[2], a[3] }; }
	std::filesystem::path Path(const std::string& name) {
		Require(TKM::ParticleEffectStore::IsValidName(name), "Use 1-64 letters, digits, _ or - for the name");
		return std::filesystem::path(TKM::ParticleEffectStore::kDirectory) / (name + ".json");
	}
	void ValidateTexture(const std::string& texture) {
		Require(std::filesystem::is_regular_file(texture), "Texture file does not exist");
	}
}

namespace TKM {
	void ParticleEffectSettings::Validate() const {
		Require(!texture.empty() && texture.size() < 512, "Invalid texture path");
		auto ext = std::filesystem::path(texture).extension().string();
		std::transform(ext.begin(), ext.end(), ext.begin(), [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
		Require(ext == ".png" || ext == ".dds" || ext == ".jpg" || ext == ".jpeg" || ext == ".bmp", "Unsupported texture extension");
		Require(shape >= 0 && shape <= 3, "Invalid shape");
		Require(count >= 1 && count <= 512, "Count must be 1-512");
		Require(InRange(interval, 0.01f, 60.0f), "Interval must be 0.01-60 seconds");
		Require(VectorInRange(spawnExtent, 0.0f, 1000.0f), "Invalid spawn extent");
		Require(VectorInRange(velocityMin, -1000.0f, 1000.0f) && VectorInRange(velocityMax, -1000.0f, 1000.0f), "Invalid velocity");
		Require(velocityMin.x <= velocityMax.x && velocityMin.y <= velocityMax.y && velocityMin.z <= velocityMax.z, "Velocity minimum exceeds maximum");
		Require(VectorInRange(gravity, -1000.0f, 1000.0f), "Invalid gravity");
		Require(InRange(lifeMin, 0.01f, 60.0f) && InRange(lifeMax, lifeMin, 60.0f), "Invalid lifetime range");
		Require(InRange(startSize, 0.0f, 1000.0f) && InRange(endSize, 0.0f, 1000.0f), "Invalid size");
		for (const auto& c : { startColor, endColor }) {
			Require(InRange(c.x, 0, 1) && InRange(c.y, 0, 1) && InRange(c.z, 0, 1) && InRange(c.w, 0, 1), "Color must be 0-1");
		}
	}

	bool ParticleEffectStore::IsValidName(const std::string& name) {
		if (name.empty() || name.size() > 64) { return false; }
		std::string upper;
		for (unsigned char c : name) {
			if (!((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9') || c == '_' || c == '-')) { return false; }
			upper += static_cast<char>(std::toupper(c));
		}
		if (upper == "CON" || upper == "PRN" || upper == "AUX" || upper == "NUL") { return false; }
		if (upper.size() == 4 && (upper.substr(0, 3) == "COM" || upper.substr(0, 3) == "LPT") && upper[3] >= '0' && upper[3] <= '9') { return false; }
		return true;
	}
	bool ParticleEffectStore::Exists(const std::string& name) {
		std::error_code error;
		return IsValidName(name) && std::filesystem::exists(Path(name), error);
	}
	std::vector<std::string> ParticleEffectStore::List() {
		std::vector<std::string> names;
		std::error_code error;
		std::filesystem::directory_iterator it(kDirectory, error), end;
		while (!error && it != end) {
			if (it->is_regular_file(error) && it->path().extension() == ".json") {
				const auto name = it->path().stem().string();
				if (IsValidName(name)) { names.push_back(name); }
			}
			it.increment(error);
		}
		std::sort(names.begin(), names.end());
		return names;
	}
	bool ParticleEffectStore::Load(const std::string& name, ParticleEffectSettings& out, std::string& error) {
		try {
			std::ifstream file(Path(name));
			Require(static_cast<bool>(file), "Cannot open effect file");
			Json j; file >> j;
			Require(j.at("version").is_number_integer() && j.at("version") == 1, "Unsupported effect version");
			ParticleEffectSettings s;
			s.texture = j.at("texture").get<std::string>();
			Require(j.at("shape").is_number_integer() && j.at("shape").get<int64_t>() >= 0 && j.at("shape").get<int64_t>() <= 3, "Invalid shape");
			s.shape = j.at("shape").get<int>();
			Require(j.at("count").is_number_integer() && j.at("count").get<int64_t>() >= 1 && j.at("count").get<int64_t>() <= 512, "Invalid count");
			s.count = j.at("count").get<uint32_t>();
			s.loop = j.at("loop").get<bool>(); s.interval = j.at("interval").get<float>();
			s.spawnExtent = Read3(j.at("spawnExtent"));
			s.velocityMin = Read3(j.at("velocityMin")); s.velocityMax = Read3(j.at("velocityMax"));
			s.gravity = Read3(j.at("gravity"));
			s.lifeMin = j.at("lifeMin").get<float>(); s.lifeMax = j.at("lifeMax").get<float>();
			s.startSize = j.at("startSize").get<float>(); s.endSize = j.at("endSize").get<float>();
			s.startColor = Read4(j.at("startColor")); s.endColor = Read4(j.at("endColor"));
			s.Validate(); ValidateTexture(s.texture);
			out = s; error.clear(); return true;
		} catch (const std::exception& e) { error = e.what(); return false; }
	}
	bool ParticleEffectStore::Save(const std::string& name, const ParticleEffectSettings& s, std::string& error) {
		try {
			const auto path = Path(name);
			s.Validate(); ValidateTexture(s.texture);
			Json j = { {"version", 1}, {"texture", s.texture}, {"shape", s.shape}, {"count", s.count},
				{"loop", s.loop}, {"interval", s.interval}, {"spawnExtent", ToJson(s.spawnExtent)},
				{"velocityMin", ToJson(s.velocityMin)}, {"velocityMax", ToJson(s.velocityMax)}, {"gravity", ToJson(s.gravity)},
				{"lifeMin", s.lifeMin}, {"lifeMax", s.lifeMax}, {"startSize", s.startSize}, {"endSize", s.endSize},
				{"startColor", ToJson(s.startColor)}, {"endColor", ToJson(s.endColor)} };
			std::filesystem::create_directories(path.parent_path());
			auto temporary = path; temporary += ".tmp";
			std::ofstream file(temporary, std::ios::trunc);
			Require(static_cast<bool>(file), "Cannot open temporary file");
			file << j.dump(4); file.close();
			Require(static_cast<bool>(file), "Cannot write effect file");
#ifdef _WIN32
			Require(MoveFileExW(temporary.c_str(), path.c_str(), MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH) != 0, "Cannot replace effect file");
#else
			std::filesystem::rename(temporary, path);
#endif
			error.clear(); return true;
		} catch (const std::exception& e) { error = e.what(); return false; }
	}
}
