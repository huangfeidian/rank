#pragma once
#include <nlohmann/json.hpp>
#include <vector>
#include <functional>

using json = nlohmann::json;


namespace spiritsaway::system::rank
{
	struct rank_info
	{
		std::string player_id;
		double rank_value = 0.0;
		std::uint64_t update_ts = 0;
		json::object_t player_info;
		NLOHMANN_DEFINE_TYPE_INTRUSIVE(rank_info, player_id, rank_value, update_ts, player_info)

		std::tuple<double, std::uint64_t, std::string_view> rank_key() const
		{
			return std::make_tuple(-1 * rank_value, update_ts, std::string_view(player_id));
		}
		bool operator<(const rank_info& other) const
		{
			return rank_key() < other.rank_key();
		}
		void clear()
		{
			player_id.clear();
			player_info.clear();
		}
	};
	struct rank_info_ptr_wrapper
	{
		const rank_info* ptr;
		bool operator<(const rank_info_ptr_wrapper& other) const
		{
			return ptr->rank_key() < other.ptr->rank_key();
		}
	};

}
