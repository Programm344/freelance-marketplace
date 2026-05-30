export interface User {
  id: number;
  email: string;
  role: 'freelancer' | 'customer' | 'moderator' | 'admin';
  created_at: string;
}

export interface FreelancerProfile {
  id: number;
  email: string;
  display_name: string;
  specialization: string;
  experience: string;
  skills: string;
  hourly_rate: number;
  rating: number;
  is_available: boolean;
  reviews: Review[];
}

export interface Order {
  id: number;
  title: string;
  description: string;
  budget: number;
  deadline: string;
  status: string;
  category: string;
  customer_email: string;
  required_skills: string;
  created_at: string;
}

export interface Response {
  id: number;
  order_id: number;
  freelancer_id: number;
  freelancer_email: string;
  message: string;
  proposed_budget: number;
  proposed_deadline: string;
  status: string;
  created_at: string;
}

export interface Review {
  id: number;
  author: string;
  rating: number;
  comment: string;
  order_title: string;
  created_at: string;
}

export interface ExternalOrder {
  id: number;
  title: string;
  description: string;
  source_name: string;
  source_url: string;
  budget: number;
  status: string;
  discovered_at: string;
}

export interface CrawlerSource {
  id: number;
  name: string;
  base_url: string;
  status: string;
  frequency: number;
  last_run_at: string;
}
