package mooltipass.automatedTest.features;

import cucumber.api.java.en.Then;
import cucumber.api.java.en.When;
import junit.framework.Assert;
import mooltipass.automatedTest.config.WebDriverFactory;
import mooltipass.automatedTest.pageObjects.Facebook;

public class FacebookSteps {
	Facebook facebook= new Facebook(WebDriverFactory.get());

	@When("I login Facebook with '(.*)'")
	public void login(String username){

		facebook.enterEmail(username);
		facebook.submit();
		String password=System.getenv().get("PASS1");
		facebook.enterPassword(password);
		facebook.submit();
		
	}
	@When("I go to Facebook login page")
	public void pressLogin(){
		Assert.assertTrue("Expected to be at login page", facebook.checkAtLoginPage());
		
	}
	@Then("I should be logged in Facebook")
	public void checkLogin(){
		Assert.assertTrue("Expected User to be logged in",facebook.checkLogin());
	}
	
	@When("I logout Facebook")
	public void pressLogout(){
		facebook.goTodDashboard();
		facebook.logout();
	}
}
