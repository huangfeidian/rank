#pragma once
#include <string>
#include <cstdint>
#include <vector>
#include <deque>
#include <unordered_map>
#include <nlohmann/json.hpp>
#include <algorithm>
#include <optional>

#include "rank_interface.h"

using json = nlohmann::json;
namespace spiritsaway::system::rank
{

	class array_rank : public rank_interface
	{

		std::vector<rank_info> m_rank_info_pool;
		std::vector<std::uint32_t> m_pool_unused_indexes;
		std::unordered_map<std::string, std::uint32_t> m_player_to_pool_idx;
		// 从大到小排序
		std::vector<rank_info_ptr_wrapper> m_sorted_rank_ptrs;

	private:
		void reset(const std::vector<rank_info> &player_ranks);

	public:
		array_rank(const std::string& name, std::uint32_t rank_sz, std::uint32_t pool_sz, double min_value, double max_value);
		const std::vector<rank_info_ptr_wrapper> &sorted_rank_ptrs() const
		{
			return m_sorted_rank_ptrs;
		}
	public:
		// overrides
		json encode() const override;
		bool decode(const json& data) override;
		bool update_player_info(const std::string& player_id, const json::object_t& player_info) override;
		bool remove(const std::string& player_id) override;
		std::uint32_t size() const override
		{
			return m_sorted_rank_ptrs.size();
		}
		std::uint32_t update(const rank_info& one_player) override;

		std::pair<const rank_info*, std::uint32_t> get_rank(const std::string& player_id) const override;

		const rank_info* get_player(std::uint32_t in_rank) const override;

		std::vector<const rank_info*> get_players(std::uint32_t begin_rank, std::uint32_t end_rank) const override;

		std::uint32_t get_rank(double rank_value) const override;
	};
}