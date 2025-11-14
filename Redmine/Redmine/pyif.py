import requests
from datetime import datetime


class RedmineClient:
    def __init__(self, base_url):
        self.base_url = base_url.rstrip('/')

    def get_issues(self, limit=100, offset=0):
        url = f"{self.base_url}/issues.json"
        params = {
            'limit': limit,  # 最多获取limit条issues
            'offset': offset  # 从第offset条开始
        }

        try:
            response = requests.get(url, params=params)
            response.raise_for_status()
            return response.json()
        except requests.exceptions.RequestException as e:
            print(f"请求错误: {e}")
            return None

    def get_all_issues_by_assignee(self, assignee_name):
        all_issues = []
        limit = 100
        offset = 0

        while True:
            issues_data = self.get_issues(limit=limit, offset=offset)
            if not issues_data:
                break

            issues = issues_data.get('issues', [])
            if not issues:
                break

            for issue in issues:
                assigned_to = issue.get('assigned_to', {})
                if assigned_to and assigned_to.get('name') == assignee_name:
                    all_issues.append(issue)

            if len(issues) < limit:  # 当前批次不满limit，说明是最后一页
                break

            offset += len(issues)

        return {
            'issues': all_issues,
            'total_count': len(all_issues),
        }

    def get_all_issues_by_assignee_as_dict(self, assignee_name):
        """
        根据分配人员获取IssueInfo对象列表
        """
        issues_data = self.get_all_issues_by_assignee(assignee_name)
        if not issues_data:
            return []

        issues_dict = []
        for issue in issues_data.get('issues', []):
            # 处理分配人员
            assigned_to = issue.get('assigned_to', {})
            assigned_to_name = assigned_to.get('name') if assigned_to else None

            # 处理项目
            project = issue.get('project', {})
            project_name = project.get('name', 'N/A')

            # 处理跟踪类型
            tracker = issue.get('tracker', {})
            tracker_name = tracker.get('name', 'N/A')

            issue_dict = {
                'id': issue.get('id', 0),
                'subject': issue.get('subject', 'N/A'),
                'status': issue.get('status', {}).get('name', 'N/A'),
                'priority': issue.get('priority', {}).get('name', 'N/A'),
                'author': issue.get('author', {}).get('name', 'N/A'),
                'assigned_to': assigned_to_name,
                'created_on': self.format_date(issue.get('created_on', 'N/A')),
                'updated_on': self.format_date(issue.get('updated_on', 'N/A')),
                'start_date': issue.get('start_date'),
                'due_date': issue.get('due_date'),
                'done_ratio': issue.get('done_ratio', 0),
                'project': project_name,
                'tracker': tracker_name,
                'description': issue.get('description', '')
            }
            issues_dict.append(issue_dict)

        return issues_dict

    def format_date(self, date_string):
        """
        格式化日期字符串
        """
        if date_string == 'N/A':
            return 'N/A'

        try:
            date_obj = datetime.fromisoformat(date_string.replace('Z', '+00:00'))
            return date_obj.strftime("%Y-%m-%d %H:%M:%S")
        except:
            return date_string


def get_issues_by_assignee_name_cpp_intf(assignee_name):
    redmine_url = "http://192.168.3.202:3000"
    client = RedmineClient(redmine_url)
    return client.get_all_issues_by_assignee_as_dict(assignee_name=assignee_name)