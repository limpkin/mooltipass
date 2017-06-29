package mooltipass.automatedTest.features;

import cucumber.api.java.en.Then;
import cucumber.api.java.en.When;
import junit.framework.Assert;
import mooltipass.automatedTest.config.WebDriverFactory;
import mooltipass.automatedTest.pageObjects.AliBaba;
public class AliBabaSteps {
	AliBaba alibaba= new AliBaba(WebDriverFactory.get());

		@When("I login AliBaba with '(.*)' and '(.*)'")
		public void login(String username,String password){
			alibaba.goTodDashboard();
			alibaba.goToLogin();
			alibaba.enterEmail(username);
			alibaba.enterPassword(password);
			alibaba.submit();
			
		}
		@When("I go to AliBaba login page")
		public void pressLogin(){
			alibaba.goTodDashboard();
			alibaba.goToLogin();
			Assert.assertFalse("Expected to be at login page", alibaba.checkAtLoginPage());
			
		}
		@When("I confirm login to Alibaba")
		public void confirmLogin(){
			alibaba.goTodDashboard();
			alibaba.goToLogin();
			alibaba.clickAccessNow();
			
		}
		@Then("I should be logged in AliBaba")
		public void checkLogin(){
			Assert.assertTrue("Expected User to be logged in",alibaba.checkLogin());
		}
		
		@When("I logout AliBaba")
		public void pressLogout(){
			alibaba.goTodDashboard();
			alibaba.logout();			
		}
}
