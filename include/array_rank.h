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

		std::unordered_map<std::string, std::unique_ptr<rank_info>> m_player_rank_infos;
		// 从大到小排序
		std::vector<rank_info_ptr_wrapper> m_sorted_rank_ptrs;

	protected:
		void reset(const std::vector<rank_info> &player_ranks) override;

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
			return std::uint32_t(m_sorted_rank_ptrs.size());
		}
		update_rank_result update(const rank_info& one_player) override;

		std::pair<const rank_info*, std::uint32_t> get_rank(const std::string& player_id) const override;

		const rank_info* get_player(std::uint32_t in_rank) const override;

		std::vector<const rank_info*> get_players(std::uint32_t begin_rank, std::uint32_t end_rank) const override;

		std::uint32_t get_rank(double rank_value) const override;
		std::string rank_impl_name() const override
		{
			return "array_rank";
		}

		static std::unique_ptr<array_rank> create(const json& data);
	};
}