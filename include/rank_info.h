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
		NLOHMANN_DEFINE_TYPE_INTRUSIVE(rank_info, player_id, rank_value, player_info, update_ts)

		std::tuple<double, std::uint64_t, std::string_view> rank_key() const
		{
			return std::make_tuple(-1 * rank_value, update_ts, std::string_view(player_id));
		}
		bool operator<(const rank_info& other) const
		{
			return rank_key() < other.rank_key();
		}
		bool operator<(double other) const
		{
			return rank_value >= other;
		}
		void clear()
		{
			player_id.clear();
			player_info.clear();
		}
		void update_value_and_ts(double new_value, std::uint64_t new_ts)
		{
			rank_value = new_value;
			update_ts = new_ts;
		}
	};
	struct rank_info_ptr_wrapper
	{
		const rank_info* ptr;
		bool operator<(const rank_info_ptr_wrapper& other) const
		{
			return ptr->rank_key() < other.ptr->rank_key();
		}
		bool operator<(double other) const
		{
			return *ptr < other;
		}
	};

}
