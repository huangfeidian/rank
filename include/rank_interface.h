#pragma once

#include "rank_info.h"
#include <optional>
namespace spiritsaway::system::rank
{
	class rank_interface
	{
	public:
		rank_interface(const std::string& name, std::uint32_t rank_sz, std::uint32_t pool_sz, double min_value, double max_value)
			: m_name(name)
			, m_rank_sz(rank_sz)
			, m_pool_sz(pool_sz)
			, m_min_value(min_value)
			, m_max_value(max_value)
			, m_update_ts(0)
		{

		}
		virtual ~rank_interface()
		{

		}

		virtual json encode() const
		{
			json result;
			result["name"] = m_name;
			result["rank_sz"] = m_rank_sz;
			result["pool_sz"] = m_pool_sz;
			result["min_value"] = m_min_value;
			result["max_value"] = m_max_value;
			return result;
		}
		virtual bool decode(const json& data) = 0;

		virtual bool update_player_info(const std::string& player_id, const json::object_t& player_info) = 0;
		virtual bool remove(const std::string& player_id) = 0;

		virtual std::uint32_t size() const = 0;
		// rank 为1 代表第一名  为0 代表不在排行榜上
		virtual std::uint32_t update(const rank_info& one_player) = 0;

		// 获取玩家对应的信息与排名
		virtual std::pair<const rank_info*, std::uint32_t> get_rank(const std::string& player_id) const = 0;
		// 获取第n名的信息
		virtual const rank_info* get_player(std::uint32_t in_rank) const = 0;
		// 获取[begin_rank, end_rank]区间内的玩家信息 
		virtual std::vector<const rank_info*> get_players(std::uint32_t begin_rank, std::uint32_t end_rank) const = 0;
		// 获取这个分数代表的同分最后一名的排名 
		virtual std::uint32_t get_rank(double rank_value) const = 0;

		std::uint64_t gen_next_update_ts()
		{
			return ++m_update_ts;
		}
	public:
		const std::string m_name;
		const std::uint32_t m_rank_sz;
		const std::uint32_t m_pool_sz;
		const bool m_min_value;
		const bool m_max_value;
	private:
		std::uint64_t m_update_ts;
	};
}